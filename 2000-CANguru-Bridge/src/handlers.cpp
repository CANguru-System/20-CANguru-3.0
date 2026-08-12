#include <WiFiUdp.h>
#include <esp_now.h>
#include "handlers.h"
#include "protocol_constants_core.h"
#include "globals.h"
#include "Bridge_Decoder.h"
#include "CAN_proc.h"
#include "alive.h"
#include "PacketHandler.h"
#include "gui_proc.h"

volatile bool thresholdAvailable = false;
volatile uint16_t lastThreshold = 0;

// ---------------------------------------------------------
// GUI START
// ---------------------------------------------------------
void handleGuiStart()
{
    guiIP = udp.remoteIP();
    systemReady = true;
    sendStartToDecoders();

}

// ---------------------------------------------------------
// PowerDecoder: GET_THRESHOLD
// ---------------------------------------------------------
void sendResetSettingsToDecoder(const uint8_t* payload, int payloadLen)
{
    // Anfrage an Decoder senden
    auto request = buildPacket(
        FEATURE_SETTING,
        SETTING_CMD_RESET_TO_DEFAULTS,
        nullptr,
        0
    );
    sendTheData(SendMode::ASSIGNED_ID, payload[0], request.data(), request.size());
}
// ---------------------------------------------------------
// PowerDecoder: GET_THRESHOLD
// ---------------------------------------------------------
void handleGetPowerDecoderSettings()
{
    // Anfrage an Decoder senden
    auto request = buildPacket(
        FEATURE_POWER,
        POWER_CMD_GET_THRESHOLD,
        nullptr,
        0
    );
    sendTheData(SendMode::TYPE, DEVTYPE_POWER, request.data(), request.size());
    Serial.println("Requested power decoder threshold");
}

// ---------------------------------------------------------
// PowerDecoder: SET_THRESHOLD
// ---------------------------------------------------------
void handleSetPowerDecoderSettings(const uint8_t* payload, int payloadLen)
{
    if (payloadLen < 2)
    {
        Serial.println("SetPowerDecoderSettings: Payload too short");
        return;
    }

    uint16_t thr = (payload[0] << 8) | payload[1];

/*    uint8_t payload[2] = {
        (uint8_t)(thr >> 8),
        (uint8_t)(thr & 0xFF)
    };*/
Serial.printf("Setting power decoder threshold to %u\r\n", thr);
    auto request = buildPacket(
        FEATURE_POWER,
        POWER_CMD_SET_THRESHOLD,
        payload,
        2
    );
    sendTheData(SendMode::TYPE, DEVTYPE_POWER, request.data(), request.size());
}

// ----------------------------------------------------
// FEEDBACK-Frames an Decoder senden (ESP-NOW)
// ----------------------------------------------------

void sendFeedbackGetSettingsToDecoder(uint8_t assignedId)
{
  uint8_t payload[1] = {assignedId};

  auto bytes = buildPacket(
      FEATURE_FEEDBACK,
      FEEDBACK_CMD_GET_SETTINGS,
      payload,
      1);
  sendTheData(SendMode::TYPE, DEVTYPE_RM, bytes.data(), bytes.size());
}

void sendFeedbackSetSettingsToDecoder(uint8_t assignedId, const uint8_t *values, int count)
{
  uint8_t pkt[1 + 16];
  pkt[0] = assignedId;

  memcpy(&pkt[1], values, 16);

  auto bytes = buildPacket(
      FEATURE_FEEDBACK,
      FEEDBACK_CMD_SET_SETTINGS,
      pkt,
      17);
  sendTheData(SendMode::TYPE, DEVTYPE_RM, bytes.data(), bytes.size());
}

void handleFeedbackGuiCommand(uint8_t cmd, const uint8_t *payload, int len)
{
  uint8_t assignedId = payload[0];

  switch (cmd)
  {
  case FEEDBACK_CMD_GET_SETTINGS:
  Serial.printf("Requesting Feedback settings from decoder with assigned ID %u\r\n", assignedId);
    sendFeedbackGetSettingsToDecoder(assignedId);
    break;

  case FEEDBACK_CMD_SET_SETTINGS:
    sendFeedbackSetSettingsToDecoder(assignedId, &payload[1], len - 1);
    break;
  }
}
