#pragma once
#include <Arduino.h>
#include <vector>

struct ParsedPacket
{
    uint8_t featureId = 0;
    uint8_t commandId = 0;
    std::vector<uint8_t> payload;
    bool valid = false;
};

ParsedPacket parsePacket(const uint8_t* data, size_t len);
std::vector<uint8_t> buildPacket(uint8_t featureId, uint8_t commandId, const uint8_t* payload, uint8_t payloadLen);
