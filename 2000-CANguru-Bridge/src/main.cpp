#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>
#include <LittleFS.h>
#include <ETH.h>
#include <globals.h>
#include <Adafruit_GFX.h>
#include "protocol_constants_core.h"
#include "otaQueue.h"
#include "wdp_proc.h"
#include "CAN_proc.h"
#include "gui_proc.h"
#include "alive.h"
#include "Bridge_OTA.h"
#include "Bridge_Decoder.h"
#include "Cs2FileReceiver.h"
#include "handlers.h"
#include "MOD-LCD.h"

// Der Master speichert im setup seine Broadcast-Adresse und jeweils bei Empfang der Slaves deren MAC-Adresse.
// Er antwortet auf den Broadcast mit einer ID-Zuweisung. Danach können die Slaves per Unicast angesprochen werden.
// Zunächst sendet er Broadcast, die Dlaves melden sich und der Master registiert dann deren Adtesse und schickt eine
// ID-Zuweisung zurück. Danach können die Slaves per Unicast angesprochen werden.

// bridge_v3_alive_v1.cpp
// ESP32-C3 Bridge v3 + Alive v1
// - ESPNOW zu Decodern
// - UDP zur GUI
// - IP-Ermittlung
// - Alive-Tracking mit DecoderInfo
// - Alive-Summary an GUI
// ----------------------------------------------------
// Konfiguration
// ----------------------------------------------------

const uint16_t GUI_PORT_IN = BRIDGE_PORT; // GUI → Bridge
const uint16_t GUI_PORT_OUT = GUI_PORT;   // Bridge → GUI

const char *CS_PATH = "/lokomotive.cs2";

// --- Forward Declarations ---
void handleFirmware();

// Forward für Routing-Funktionen
void sendSwitchSignalSettingsToDecoder(uint8_t commandId, const uint8_t *payload, int payloadLen);

void initLittleFS()
{
  if (!LittleFS.begin(false))
  {
    Serial.println("LittleFS not mounted → formatting...");
    if (!LittleFS.begin(true))
    {
      Serial.println("LittleFS formatting FAILED!");
      return;
    }
    Serial.println("LittleFS formatted successfully.");
  }
  else
  {
    Serial.println("LittleFS mounted successfully.");
  }
}

// --------------------------------------------------
// setup()
// --------------------------------------------------
void setup()
{
#if defined ARDUINO_ESP32_EVB
  delay(500);
#endif
  Serial.begin(bdrMonitor);
  delay(500);

  log_i("\r\n\r\nC A N g u r u - B r i d g e");
  log_i("\n on %s", ARDUINO_BOARD);
  log_i("CPU Frequency = %d Mhz", F_CPU / 1000);
  initDisplayLCD28();

  displayLCD("CANguru-Release-Version");

  // die Routine für die Statusmeldungen des WiFi wird registriert
  wifi_event_id_t inet_evt_hnd = WiFi.onEvent(iNetEvtCB);

  ETH.begin();

  while (!ETH.linkUp())
    delay(100);
  while (ETH.localIP().toString() == "0.0.0.0")
    delay(100);

  Serial.print("Bridge IP: ");
  Serial.println(ETH.localIP());

  udpWDP.begin(WDP_PORT_IN);
  udp.begin(GUI_PORT_IN);
  initWdpQueue();

  xTaskCreate(wdpSendTask, "wdpSendTask", 4096, NULL, 1, NULL);

  InitAlive();
  initTWAI();

  // WiFi + ESP-NOW initialisieren
  initBridgeEspNow();

  setupDownloadRoutesCS2();
  setupUploadRoute();
  otaServer.begin();

  // LittleFS mounten
  initLittleFS();

  if (!LittleFS.exists(CS_PATH))
    Serial.println("BRIDGE: WARNING - lokomotive.cs2 not found in LittleFS");

  Serial.println("Bridge ready for HELLO packets");

  // Decoder-Liste initialisieren
  //  loadDecoderList();
}

// ----------------------------------------------------
// Routing für neue Protokoll-Frames (Feature | Frame | Payload)
// ----------------------------------------------------
void handleGuiPacket(uint8_t featureId, uint8_t commandId, const uint8_t *payload, int payloadLen)
{

  switch (featureId)
  {
  case FEATURE_POWER:
    switch (commandId)
    {
    case POWER_CMD_GET_THRESHOLD:
      handleGetPowerDecoderSettings();
      break;
    case POWER_CMD_SET_THRESHOLD:
      handleSetPowerDecoderSettings(payload, payloadLen);
      break;
    default:
      Serial.printf("Unknown POWER commandId %u\r\n", commandId);
      break;
    }
    break;

  case FEATURE_FEEDBACK:
    handleFeedbackGuiCommand(commandId, payload, payloadLen);
    break;

  case FEATURE_SWITCH:
  case FEATURE_SIGNAL:
    switch (commandId)
    {
    case SWITCH_CMD_SET_SETTINGS:
    case SWITCH_CMD_GET_SETTINGS:
    case SWITCH_CMD_MOVE2START:
    case SWITCH_CMD_SET_ZERO:
    case SWITCH_CMD_MOVE2END:
    case SWITCH_CMD_SET_END:
    case SWITCH_CMD_TEST_RUN:
    case SWITCH_CMD_STOP_ALL:
    case SIGNAL_CMD_SET_SETTINGS:
    case SIGNAL_CMD_GET_SETTINGS:
    case SIGNAL_CMD_MOVE2START:
    case SIGNAL_CMD_SET_ZERO:
    case SIGNAL_CMD_MOVE2END:
    case SIGNAL_CMD_SET_END:
    case SIGNAL_CMD_TEST_RUN:
    case SIGNAL_CMD_STOP_ALL:
      sendSwitchSignalSettingsToDecoder(commandId, payload, payloadLen);
      break;

      // andere Features …

    default:
      Serial.printf("Unknown SWITCH commandId %02X\r\n", commandId);
      break;
    }
    break;
  case FEATURE_SETTING:
    switch (commandId)
    {
    case SETTING_CMD_RESET_TO_DEFAULTS:
      sendResetSettingsToDecoder(payload, payloadLen);
      break;

    default:
      Serial.printf("Unknown SETTING commandId %u\r\n", commandId);
      break;
    }
    break;
  case FEATURE_BRIDGE:
    switch (commandId)
    {
    case BRIDGE_START:
      handleGuiStart();
      break;

    default:
      Serial.printf("Unknown BRIDGE commandId %u\r\n", commandId);
      break;
    }
    break;

  case FEATURE_DECODER:
    switch (commandId)
    {
    case CMD_DECODER_RESET:
    {
      enqueueCANFrame2CAN(SysSTOPP, hashEnabled);
      delay(1000);
      sendResetToDecoders();
      ESP.restart();
      break;
    }
    }
    break;

  case FEATURE_OTA:
    switch (commandId)
    {
    case OTA_SINGLE_DECODER:
    case OTA_GROUP_DECODERS:
      otaHandleGuiCommand(payload, payloadLen, commandId);
      break;
    }
    break;

  default:
    Serial.printf("Unknown DECODER featureId %02X\r\n", featureId);
    break;
  }
}

// ----------------------------------------------------
// GUI-Pakete lesen
// ----------------------------------------------------
void readGUI()
{
  int packetLength = udp.parsePacket();
  if (packetLength <= 0)
    return;

  lastGuiSeen = millis();

  uint8_t buf[BlockSizeBridge];
  if (packetLength > (int)sizeof(buf))
    packetLength = sizeof(buf);
  udp.read(buf, packetLength);

  // Sonderformat für OTA-Firmware-Blöcke:
  // OTA-Block?
  if (buf[0] == OTA_UPLOAD_BLOCK)
  {
    int otaLen = packetLength; // <-- WICHTIG: echte UDP-Länge
    if (otaLen < 9)
    {
      Serial.println("BRIDGE: OTA frame too short");
      return;
    }

    uint32_t seq = *(uint32_t *)&buf[1];
    uint16_t plen = *(uint16_t *)&buf[5];
    uint16_t crc = *(uint16_t *)&buf[7];
    uint8_t *payload = &buf[9];

    // Startblock
    if (seq == 0)
    {
      if (plen < 4)
      {
        Serial.println("FW: Startblock ohne gültige Größe!");
        AckPacket a = {(uint16_t)(seq & 0xFFFF), 2};
        ackQueue.push(a);
        return;
      }

      expectedSize = *(uint32_t *)payload;
      receivedSize = 0;
      firmwareReady = false;

      if (fwFile)
        fwFile.close();

      LittleFS.remove(FW_PATH);
      fwFile = LittleFS.open(FW_PATH, FILE_WRITE);

      if (!fwFile)
      {
        Serial.println("FW: Kann firmware.bin nicht öffnen!");
        AckPacket a = {(uint16_t)(seq & 0xFFFF), 1};
        ackQueue.push(a);
        return;
      }

      AckPacket a = {(uint16_t)(seq & 0xFFFF), 0};
      ackQueue.push(a);
      return;
    }

    // Datenblock
    if (!fwFile)
    {
      Serial.println("FW: Datenblock ohne Startblock!");
      AckPacket a = {(uint16_t)(seq & 0xFFFF), 2};
      ackQueue.push(a);
      return;
    }

    size_t written = fwFile.write(payload, plen);
    receivedSize += written;

    AckPacket a = {(uint16_t)(seq & 0xFFFF), (written == plen) ? 0 : 1};
    ackQueue.push(a);

    if (expectedSize > 0 && receivedSize >= expectedSize)
    {
      fwFile.close();
      firmwareReady = true;
      Serial.println("FW: Upload abgeschlossen");

      onUploadFinished(receivedSize); // <-- HIER
    }

    return;
  }

  // Sonderfall: CAN-Frame direkt von GUI
  if (packetLength == CAN_FRAME_SIZE && buf[0] == 0x00)
  {
    enqueueCANFrame2CAN(buf, hashEnabled);
    if (buf[0x00] == 0x00 && buf[0x01] == PING)
    {
      sendToWDP(buf);
      sendFrame2allDecoders(buf, packetLength);
    }
    return;
  }

  if (buf[0] == CMD_FILE_BLOCK_CS2)
  {
    Cs2FileReceiver::handleFileBlock(buf, packetLength);
    return;
  }

  // ----------------------------------------------------
  // 4. NEUES GUI-Paketformat:
  //    Feature | Frame | Payload
  // ----------------------------------------------------
  if (packetLength >= 2)
  {
    uint8_t feature = buf[0];
    uint8_t frame = buf[1];

    const uint8_t *payload = nullptr;
    int payloadLen = packetLength - 2;

    if (payloadLen > 0)
      payload = &buf[2];

    handleGuiPacket(feature, frame, payload, payloadLen);
  }

  //  onUdpData(buf, packetLength);
}

// ----------------------------------------------------
// SWITCH-Frames an Decoder senden (ESP-NOW)
// ----------------------------------------------------

void sendSwitchSignalSettingsToDecoder(uint8_t commandId, const uint8_t *payload, int payloadLen)
{
  if (payloadLen == 0)
  {
    Serial.println("ERROR: SWITCH packet without payload");
    sendGuiOtaError(0x05);
    return;
  }

  auto bytes = buildPacket(
      FEATURE_SWITCH,
      commandId,
      payload,
      payloadLen);
  sendTheData(SendMode::TYPE, DEVTYPE_SWITCH, bytes.data(), bytes.size());
}

// ----------------------------------------------------
// Hilfsfunktion: lokomotive.cs2 anzeigen
// ----------------------------------------------------
void showLokomotiveFile()
{
  File f = LittleFS.open("/lokomotive.cs2", "r");
  if (!f)
  {
    Serial.println("Fehler: lokomotive.cs2 konnte nicht geöffnet werden!");
    return;
  }

  Serial.println("===== Inhalt von lokomotive.cs2 =====");

  while (f.available())
  {
    String line = f.readStringUntil('\n');
    Serial.println(line);
  }

  Serial.println("=====================================");
  f.close();
}

// --------------------------------------------------
// loop()
// --------------------------------------------------
void loop()
{
  processAssignRetries();
  cleanupFinishedAssigns();
  // 0. Nachrichten von GUI lesen
  readGUI();

  // 1. CAN zuerst – höchste Priorität
  readCANMsg();

  // 2. UDP lesen (GUI / WDP)
  readWDP();

  unsigned long now = millis();
  if (now - lastRequest > 100)
  {
    lastRequest = now;
    processCANFrames2CAN();
  }

  // Alive-Timeouts + Summary
  if (millis() - lastAliveSummary > ALIVE_SUMMARY_INTERVAL_MS)
  {
    if (GUIipIsNotSet())
      return;

    lastAliveSummary = millis();
    checkAliveTimeouts();
    sendAliveSummary();
  }

  // 4. CAN erneut lesen – falls Antworten kamen
  readCANMsg();

  // 5. CAN → UDP Pipeline
  sendFiFoFromCANToGUI();

  // 6. firmware-upload-Queue verarbeiten
  processFirmwareQueue();
  processOta();
  processAckQueue();

  // 7. Alive-Überwachung + Alive-Senden
  checkAliveStates();
}
