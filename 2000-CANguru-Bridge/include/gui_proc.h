#pragma once

#include <Arduino.h>
#include <PacketHandler.h>


struct struct_frame_GUI_ota_finished
{
    uint8_t cmd; // z.B. 0x32
};

void udpSendToGui(const uint8_t* data, int len);
void sendGuiOtaStart(uint16_t totalBlocks);
void sendGuiOtaProgress(uint8_t id, uint8_t percent);
void sendGuiOtaDone();
void sendGuiOtaError(uint8_t err);
void sendGuiOtaOverallProgress(uint8_t percent);
void sendFirmwareAck(uint16_t seq, uint8_t status);
void sendGuiOtaFinished();
void sendGuiOtaCurrentDecoder(uint8_t decoderId);
void sendPacketToGUI(const std::vector<uint8_t> &bytes);
void sendPacketToGUI(const ParsedPacket &p);
void sendAck(uint32_t seq, uint8_t status);
void sendOtaAck(uint16_t seq, uint8_t status);
