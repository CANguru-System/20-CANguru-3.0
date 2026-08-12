#pragma once
#include <stdint.h>

struct AlivePacket
{
    uint8_t type;        // FRAME_DECODER_ALIVE
    uint8_t assignedId;  // Pflicht
    uint8_t status;      // Pflicht
    uint8_t featureLen;  // Länge der Feature-Daten
    uint8_t featureData[]; // optional
} __attribute__((packed));

void aliveInit();
void aliveLoop();
void aliveHandleAck(const uint8_t* data, int len);

void aliveSetDecoderID(uint8_t id);
uint8_t aliveGetDecoderID();
