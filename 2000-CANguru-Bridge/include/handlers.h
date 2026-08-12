#pragma once
#include "PacketHandler.h"

extern IPAddress guiIP;     // GUI-IP, die du schon hast
extern WiFiUDP udp;         // dein bestehender UDP-Socket
extern volatile bool thresholdAvailable;
extern volatile uint16_t lastThreshold;


void handleGuiStart();
void handleGetPowerDecoderSettings();
void handleSetPowerDecoderSettings(const uint8_t* payload, int payloadLen);
void sendResetSettingsToDecoder(const uint8_t* payload, int payloadLen);
void handleFeedbackGuiCommand(uint8_t cmd, const uint8_t *payload, int len);
