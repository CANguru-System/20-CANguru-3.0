#include <Arduino.h>
#include "protocol_constants_core.h"
#include "alive.h"
#include "globals.h"
#include "globals.h"
#include "gui_proc.h"
/**
 * The function generates a hash value from a CAN-UID by performing bitwise operations and setting
 * specific bits.
 */
#include "PacketHandler.h"   // parsePacket + buildPacket

void sendPacketToGUI(const std::vector<uint8_t> &bytes)
{
    if (GUIipIsNotSet())
        return;

    udpSendToGui(bytes.data(), bytes.size());
}

void sendPacketToGUI(const ParsedPacket &p)
{
    if (GUIipIsNotSet())
        return;

    auto bytes = buildPacket(
        p.featureId,
        p.commandId,
        p.payload.data(),
        p.payload.size()
    );

    udpSendToGui(bytes.data(), bytes.size());
}

// ⭐ 12. udpSendToGui()
void udpSendToGui(const uint8_t *data, int len)
{
  if (GUIipIsNotSet())
    return;

  udp.beginPacket(guiIP, GUI_PORT);
  udp.write(data, len);
  udp.endPacket();
}

// =========================
// GUI-Statusmeldungen
// =========================

void sendGuiOtaProgress(uint8_t id, uint8_t percent)
{
    uint8_t payload[2];
    payload[0] = percent;
    payload[1] = id;

    auto pkt = buildPacket(
        FEATURE_DECODER_UPLOAD,
        DECODER_UPLOAD_BLOCK,
        payload,
        sizeof(payload)
    );
    udpSendToGui(pkt.data(), pkt.size());
}

void sendGuiOtaDone()
{
  uint8_t payload[1] = {0x00};

    auto pkt = buildPacket(
        FEATURE_DECODER_UPLOAD,
        DECODER_UPLOAD_END,
        payload,
        sizeof(payload)
    );

    udpSendToGui(pkt.data(), pkt.size());

}

void sendGuiOtaError(uint8_t err)
{
  uint8_t payload[1] = {err};
  auto pkt = buildPacket(
      FEATURE_DECODER_UPLOAD,
      DECODER_UPLOAD_ERROR,
      payload,
      sizeof(payload)
  );
  udpSendToGui(pkt.data(), pkt.size());
}

void sendGuiOtaOverallProgress(uint8_t percent)
{
  uint8_t payload[1] = {percent};
  auto pkt = buildPacket(
      FEATURE_SHOW_BAR,
      FRAME_GUI_OTA_OVERALL,
      payload,
      sizeof(payload)
  );
  udpSendToGui(pkt.data(), pkt.size());
}

void sendFirmwareAck(uint16_t seq, uint8_t status)
{
    uint8_t payload[3];
    payload[0] = (seq >> 8) & 0xFF;
    payload[1] = seq & 0xFF;
    payload[2] = status;

    auto pkt = buildPacket(
        FEATURE_DECODER_UPLOAD,
        CMD_ACK,
        payload,
        sizeof(payload)
    );

    udpSendToGui(pkt.data(), pkt.size());
}

void sendGuiOtaFinished()
{
  uint8_t payload[1] = {0x00};
  auto pkt = buildPacket(
      FEATURE_FIRMWARE,
      FRAME_GUI_OTA_CURRENT_FINISHED,
      payload,
      sizeof(payload)
  );
  udpSendToGui(pkt.data(), pkt.size());
}

void sendGuiOtaCurrentDecoder(uint8_t decoderId)
{
  uint8_t payload[1] = {decoderId};
  auto pkt = buildPacket(
      FEATURE_SHOW_BAR,
      FRAME_GUI_OTA_CURRENT_DECODER,
      payload,
      sizeof(payload)
  );
  udpSendToGui(pkt.data(), pkt.size());
}

void sendAck(uint32_t seq, uint8_t status)
{
    uint8_t payload[5];

    // Sequence Number (4 Byte, little endian)
    payload[0] = (seq >> 0) & 0xFF;
    payload[1] = (seq >> 8) & 0xFF;
    payload[2] = (seq >> 16) & 0xFF;
    payload[3] = (seq >> 24) & 0xFF;

    // Status (0 = OK, 1 = retry)
    payload[4] = status;

    auto pkt = buildPacket(
        FEATURE_DECODER_UPLOAD,
        CMD_ACK,
        payload,
        sizeof(payload)
    );

    udpSendToGui(pkt.data(), pkt.size());
}

void sendOtaAck(uint16_t seq, uint8_t status)
{
    uint8_t pkt[3];
    pkt[0] = seq & 0xFF;
    pkt[1] = (seq >> 8) & 0xFF;
    pkt[2] = status;

    auto answer = buildPacket(
        FEATURE_OTA,
        CMD_OTA_ACK,
        pkt,
        sizeof(pkt)
    );

    udpSendToGui(answer.data(), answer.size());
}
