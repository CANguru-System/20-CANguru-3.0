#pragma once
#include <stdint.h>

class Cs2FileReceiver
{
public:
    static void handleFileBlock(const uint8_t* data, int len);

private:
    static void handlePacket(const uint8_t* data, int len);
    static void sendAck(uint32_t offset, uint8_t status);
};
