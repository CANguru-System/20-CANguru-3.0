#include <Arduino.h>
#include "protocol_constants_core.h"
#include "globals.h"

#pragma once

extern bool otaRunning;

bool findDecoderByAssignedId(uint8_t id, DecoderInfo &out);
bool findDecoderByDecoderType(uint8_t type, DecoderInfo &out);
void initBridgeEspNow();
void sendFrame2allDecoders(uint8_t *data, int len);
void sendFrame2TypeDecoders(uint8_t *data, int len, uint8_t decoderType);
void sendConfigData(const uint8_t *data);

void sendStartToDecoders();
void sendResetToDecoders();

enum class SendMode : uint8_t {
    GLOBAL,         // an alle Decoder
    TYPE,           // an alle Decoder eines Typs
    ASSIGNED_ID,    // an genau einen Decoder
    MAC_DIRECT      // nur für ID_ASSIGN
};

void sendTheData(SendMode mode, uint8_t selector, const uint8_t *data, size_t len);
