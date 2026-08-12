#pragma once
#include <nvs.h>
#include <nvs_flash.h>
#include <esp_err.h>
#include <Arduino.h>
#include "Core_EspNow.h"

class DecoderReset
{
public:
    // Löscht den NVS‑Namespace und ruft anschließend loadDefaults() auf.
static bool performReset(const char* nvsNamespace,
                         void (*loadDefaultsCallback)(),
                         uint8_t featureId,
                         uint8_t ackCommandId,
                         uint8_t assignedId,
                         uint8_t decoderType)
{
    if (!eraseNamespace(nvsNamespace))
        return false;

    if (loadDefaultsCallback)
        loadDefaultsCallback();

    sendAck(featureId, ackCommandId, assignedId, decoderType);
    return true;
}

private:
    static bool eraseNamespace(const char *nvsNamespace)
    {
        nvs_handle handle;
        esp_err_t err = nvs_open(nvsNamespace, NVS_READWRITE, &handle);
        if (err != ESP_OK)
            return false;

        err = nvs_erase_all(handle);
        if (err != ESP_OK)
        {
            nvs_close(handle);
            return false;
        }

        nvs_commit(handle);
        nvs_close(handle);
        return true;
    }

    static void sendAck(uint8_t featureId,
                        uint8_t ackCommandId,
                        uint8_t assignedId,
                        uint8_t decoderType)
    {
        uint8_t payload[2];
        payload[0] = assignedId;
        payload[1] = decoderType;

        gDecoder->sendToBridge(
            featureId,
            ackCommandId,
            payload,
            sizeof(payload));
    }
};
