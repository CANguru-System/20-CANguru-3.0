#include <Arduino.h>
#include <esp_now.h>
#include "Core_Alive.h"
#include "Core_EspNow.h"
#include "DecoderBase.h"
#include "Core_Config.h"
#include "core_globals.h"
#include "DecoderBase.h"
#include "protocol_constants_core.h"

extern bool otaInProgress;
extern bool bridge_is_known;

static uint8_t decoderID = INVALID_ASSIGNED_ID;

static unsigned long lastAlive = 0;
static const unsigned long aliveInterval = 1000;
static bool aliveAckPending = false;
static unsigned long aliveAckTimestamp = 0;

void aliveSetDecoderID(uint8_t id)
{
    decoderID = id;
}

uint8_t aliveGetDecoderID()
{
    return decoderID;
}

void aliveInit()
{
    lastAlive = millis();
    aliveAckPending = false;
}

void sendAlive()
{
    uint8_t decoderId = aliveGetDecoderID();
    if (decoderId == INVALID_ASSIGNED_ID)
        return;

    uint8_t status = (gDecoder->getStatus() == DecoderBase::AliveState::Online) ? 1 : 0;

    uint8_t featureBuf[16];
    uint8_t featureLen = gDecoder->getAliveFeatureData(featureBuf);

    uint8_t payload[32];
    uint8_t pos = 0;

    payload[pos++] = decoderId;
    payload[pos++] = DEVTYPE;
    payload[pos++] = status;
    payload[pos++] = VERS_HIGH;
    payload[pos++] = VERS_LOW;
    memcpy(&payload[pos], featureBuf, featureLen);
    pos += featureLen;
    gDecoder->sendToBridge(FEATURE_ALIVE, FRAME_DECODER_ALIVE, payload, pos);
}

void aliveLoop()
{
    static unsigned long lastAlive = 0;
    unsigned long now = millis();

    if (now - lastAlive >= aliveInterval)
    {
        lastAlive = now;
        if (bridge_is_known)
            sendAlive();
    }
}

void aliveHandleAck(const uint8_t *data, int len)
{
    if (len < 2)
        return;

    uint8_t id = data[1];
    if (id != aliveGetDecoderID())
        return;

    aliveAckPending = false;
}
