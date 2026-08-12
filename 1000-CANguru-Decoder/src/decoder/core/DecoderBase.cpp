#include <Arduino.h>
#include <esp_now.h>
#include "DecoderBase.h"
#include "Core_Logging.h"
#include "Core_Config.h"
#include "Core_EspNow.h"
#include "Core_Alive.h"
#include "Core_Ota.h"

// der zentrale Decoder‑Parser
bool DecoderBase::coreEspNowHandleInComingData(const uint8_t *raw, int len)
{
    /*
    * raw[0] = SendMode
    * raw[1] = Selector
    * raw[2] = FeatureID
    * raw[3] = CommandID
    /*
    Protokoll der empfangenen Daten:
    [0] SendMode
    [1] Selector
    [2] Feature
    [3] Frame
    [4..] Payload
*/
    if (len < 2)
        return false;

    SendMode mode = (SendMode)raw[0];
    uint8_t selector = raw[1];

    const uint8_t *payload = &raw[2];
    int payloadLen = len - 2;

    if (payloadLen < 2)
        return false;

    uint8_t featureID = payload[0];
    uint8_t commandID = payload[1];
    const uint8_t *frameData = &payload[2];
    int frameDataLen = payloadLen - 2;

    return dispatchInComingData(mode, selector, featureID, commandID, frameData, frameDataLen);
}

bool DecoderBase::dispatchInComingData(
    SendMode mode,
    uint8_t selector,
    uint8_t featureID,
    uint8_t commandID,
    const uint8_t *data,
    int len)
{ /*
Protokoll der empfangenen Daten:
[0] SendMode
[1] Selector
[2] FeatureID
[3] CommandID
[4..] Payload
*/

    switch (mode)
    {
    case SendMode::GLOBAL:
        return handleGlobalFrame(featureID, commandID, data, len);

    case SendMode::MAC_DIRECT:
        return handleMacDirectFrame(featureID, commandID, data, len);

    case SendMode::TYPE:
        if (selector == DEVTYPE)
            return handleTypedFrame(featureID, commandID, data, len);
        return false;

    case SendMode::ASSIGNED_ID:
        if (selector == aliveGetDecoderID())
            return handleIdBoundFrame(featureID, commandID, data, len);
        return false;
    }

    return false;
}
void DecoderBase::sendToBridge(uint8_t featureID, uint8_t commandID,
                               const uint8_t *payload, uint8_t len)
{
    uint8_t buffer[250];
    buffer[0] = featureID;
    buffer[1] = commandID;

    if (len > 0)
        memcpy(&buffer[2], payload, len);
    esp_err_t s = esp_now_send(Bridge_Mac, buffer, len + 2);
    if (s != ESP_OK)
    {
        Serial.printf("BRIDGE: esp_now_send FAILED: %d\r\n", s);
        return;
    }
}

void DecoderBase::setup()
{
    coreLoggingInit();
    coreConfigInit();
    coreOtaInit();
    featureSetup();

    // GANZ AM ENDE:
    coreEspNowInit(); // ESPNOW final initialisieren

    // Jetzt ist ESPNOW stabil → jetzt darf gesendet werden
    if (aliveGetDecoderID() != INVALID_ASSIGNED_ID)
        sendRejoin();
}

void DecoderBase::loop()
{
    coreOtaLoop();
    featureLoop();
}

void DecoderBase::onEspNowDataReceived(const uint8_t *data, int len)
{
    if (coreEspNowHandleInComingData(data, len))
        return;
}
