#include <Arduino.h>
#include <LittleFS.h>
#include <esp_now.h>
#include "protocol_constants_core.h"
#include "globals.h"
#include "otaQueue.h"
#include "gui_proc.h"
#include "Bridge_OTA.h"
#include "Bridge_Decoder.h"

void otaHandleGuiCommand(const uint8_t *data, int len, uint8_t cmd);

struct DecoderAck
{
  uint16_t seq;
  uint8_t status;
  uint8_t mac[6];
};

static DecoderAck lastAck;
static volatile bool ackAvailable = false;
static volatile bool doneReceived = false;

// Firmware-Upload-Status
size_t expectedSize = 0;
size_t receivedSize = 0;

std::queue<FirmwarePacket> fwQueue;
std::queue<AckPacket> ackQueue;

// uint16_t nextSeqExpected = 0;
bool uploadActive = false;
uint8_t currentDecoderId = 0;
uint8_t currentDecoderPercent = 0;

extern bool otaRunning;
static uint8_t otaTargetId = 0; // AssignedId des Decoders

void startOtaHttpServer()
{
  otaServer.on(FW_PATH, HTTP_GET, [](AsyncWebServerRequest *request)
               {
    Serial.println("BRIDGE: /firmware.bin handler reached");

    File *fp = new File(LittleFS.open(FW_PATH, "r"));
    if (!(*fp))
    {
        Serial.println("BRIDGE: Cannot open firmware.bin");
        delete fp;
        request->send(500, "text/plain", "Cannot open firmware");
        return;
    }

    size_t total = fp->size();
    Serial.printf("BRIDGE: Serving firmware.bin (%u bytes)\n", total);

    AsyncWebServerResponse *response =
        request->beginResponse("application/octet-stream", total,
            [fp, total](uint8_t *buffer, size_t maxLen, size_t index) -> size_t
            {
                size_t n = fp->read(buffer, maxLen);

                // Fortschritt berechnen
                size_t sent = index + n;
                uint8_t percent = (uint8_t)((sent * 100.0f) / total);

                // GUI informieren
                sendGuiOtaProgress(currentOta.decoderId, percent);

                // Gesamtfortschritt
                uint8_t overall = (uint8_t)(
                    ((otaFinishedCount * 100.0f) + percent) / otaTotalCount
                );
                sendGuiOtaOverallProgress(overall);

                // Datei am Ende schließen
                if (n == 0)
                {
                    fp->close();
                    delete fp;
                }

                return n;
            });

    request->send(response); });
}

void stopOtaHttpServer()
{
  otaServer.end();
}

void handleFirmwareBlockGUI2Bridge(uint8_t *data, int len)
{
  // Mindestlänge prüfen:
  // [0]   = OTA_UPLOAD_BLOCK
  // [1]   = SEQ_LO
  // [2]   = SEQ_HI
  // [3..6]= TOTAL_SIZE (4 Byte)
  // [7..] = PAYLOAD
  if (len < 7)
    return;

  uint8_t cmd = data[0];
  if (cmd != OTA_UPLOAD_BLOCK)
    return;

  // 16-Bit Sequenznummer
  uint16_t seq = (uint16_t(data[1]) | (uint16_t(data[2]) << 8));

  // Gesamtgröße (4 Byte, little endian)
  size_t totalSize =
      (size_t(data[3])) |
      (size_t(data[4]) << 8) |
      (size_t(data[5]) << 16) |
      (size_t(data[6]) << 24);

  uint8_t *payload = data + 7;
  int plen = len - 7;

  // Upload-Start?
  if (!uploadActive)
  {
    uploadActive = true;
    expectedSize = totalSize;
    receivedSize = 0;
    firmwareReady = false;
    firmwareSizeOnFs = 0;

    LittleFS.remove(FW_PATH);
    fwFile = LittleFS.open(FW_PATH, FILE_WRITE);

    if (!fwFile)
    {
      Serial.println("FW: Kann firmware.bin nicht öffnen!");

      AckPacket a;
      a.seq = seq;
      a.status = 1; // RETRY
      ackQueue.push(a);

      uploadActive = false;
      return;
    }
  }

  // ACK vorbereiten (erstmal OK)
  AckPacket a;
  a.seq = seq;
  a.status = 0;

  // Block schreiben
  size_t written = fwFile.write(payload, plen);
  receivedSize += written;

  if (written != (size_t)plen)
  {
    // Schreibfehler → RETRY
    a.status = 1;
    ackQueue.push(a);
    return;
  }

  // ACK OK senden
  ackQueue.push(a);

  // Fertig?
  if (receivedSize >= expectedSize)
  {
    fwFile.close();
    uploadActive = false;

    File f = LittleFS.open(FW_PATH, "r");
    firmwareSizeOnFs = f ? f.size() : 0;
    if (f)
      f.close();

    firmwareReady = (firmwareSizeOnFs == expectedSize);

    Serial.printf(
        "Firmware-Upload abgeschlossen. Received: %u bytes, Expected: %u bytes, On FS: %u bytes, Ready: %s\r\n",
        receivedSize, expectedSize, firmwareSizeOnFs,
        firmwareReady ? "YES" : "NO");
  }
}

// =========================
// ACK / DONE aus ESP-NOW
// =========================

void otaHandleDecoderAck(const uint8_t *mac, const uint8_t *data, int len)
{
  if (len < 4)
    return;
  if (data[0] != OTA_FW_ACK)
    return;

  memcpy(lastAck.mac, mac, 6);
  lastAck.seq = (data[1] << 8) | data[2];
  lastAck.status = data[3];

  ackAvailable = true; // letzter Schritt
}

void otaHandleDecoderDone(const uint8_t *mac, const uint8_t *data, int len)
{
  if (len < 1)
    return;
  if (data[0] != OTA_FW_DONE)
    return;

  doneReceived = true;
}

void otaHandleGuiCommand(const uint8_t *data, int len, uint8_t cmd)
{
  if (len < 2)
    return;

  uint8_t id = data[0];
  uint8_t type = data[1];

  switch (cmd)
  {
  case OTA_SINGLE_DECODER:
  {
    DecoderInfo d;
    if (!findDecoderByAssignedId(id, d))
    {
      sendGuiOtaError(0x05);
      return;
    }

    otaQueue.clear();

    OtaEntry e;
    e.decoderId = d.assignedId;
    memcpy(e.mac, d.Decoder_Mac, 6);

    otaQueue.push_back(e);
    otaTotalCount = 1;
    otaFinishedCount = 0;

    Serial.printf("OTA_SINGLE_DECODER for single decoder ID %d\r\n", id);

    if (!firmwareReady)
    {
      Serial.println("FW: OTA-Start angefordert, aber Firmware noch nicht bereit!");
      otaState = OTA_WAIT_FOR_FIRMWARE;
    }
    else
    {
      otaState = OTA_START;
    }
    break;
  }

  case OTA_GROUP_DECODERS:
  {
    startGroupOtaForType(type);

    if (otaQueue.size() == 0)
    {
      sendGuiOtaError(0x06);
      return;
    }

    otaTotalCount = otaQueue.size();
    otaFinishedCount = 0;

    otaState = OTA_START;
    break;
  }
  }
}

// ------------------------------------------------------------
// OTA_START_WIFI senden
// ------------------------------------------------------------

bool sendOtaStartWifiToDecoder(uint8_t decoderId, uint8_t fwMajor, uint8_t fwMinor)
{
  // die Versionsnummer sind zukünftigen Versionen vorbehalten, aktuell immer 0.0

  uint8_t payload[2];
  payload[0] = fwMajor;
  payload[1] = fwMinor;
  auto pkt = buildPacket(FEATURE_OTA, OTA_START_WIFI, payload, sizeof(payload));

  sendTheData(SendMode::ASSIGNED_ID, decoderId, pkt.data(), pkt.size());
  return true;
}
