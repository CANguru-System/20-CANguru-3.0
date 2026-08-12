#include <Arduino.h>
#include <queue>
#include "protocol_constants_core.h"

#pragma once


struct FirmwarePacket
{
  int len;
  uint8_t data[BlockSizeBridge];
};
extern std::queue<FirmwarePacket> fwQueue;

struct AckPacket
{
  uint16_t seq;
  uint8_t status;
};

extern std::queue<AckPacket> ackQueue;

extern uint8_t currentDecoderId;
extern uint8_t currentDecoderPercent;
extern bool uploadActive;
extern size_t expectedSize;
extern size_t receivedSize;


void handleFirmwareBlockGUI2Bridge(uint8_t *data, int len);
void startOtaHttpServer();
void stopOtaHttpServer();
bool sendOtaStartWifiToDecoder(uint8_t decoderId, uint8_t fwMajor, uint8_t fwMinor);

// =========================
// ACK / DONE aus ESP-NOW
// =========================

void otaHandleDecoderAck(const uint8_t *mac, const uint8_t *data, int len);
void otaHandleDecoderDone(const uint8_t *mac, const uint8_t *data, int len);
void otaHandleGuiCommand(const uint8_t *data, int len, uint8_t cmd);
