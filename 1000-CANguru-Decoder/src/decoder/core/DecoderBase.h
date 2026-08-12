#pragma once
#include <stdint.h>

enum class SendMode : uint8_t
{
    GLOBAL,      // an alle Decoder
    TYPE,        // an alle Decoder eines Typs
    ASSIGNED_ID, // an genau einen Decoder
    MAC_DIRECT   // nur für ID_ASSIGN
};

class DecoderBase
{
public:
    enum AliveState
    {
        Offline,
        Weak,
        Online
    };

    virtual ~DecoderBase() {}

    virtual void setup() = 0;
    virtual void loop() = 0;

    // Wird von Core_EspNow aufgerufen
    void onEspNowDataReceived(const uint8_t *data, int len);
    bool coreEspNowHandleInComingData(const uint8_t *data, int len);

    bool dispatchInComingData(
        SendMode mode,
        uint8_t selector,
        uint8_t feature,
        uint8_t frame,
        const uint8_t *data,
        int len);

    void sendToBridge(uint8_t feature, uint8_t frame,
                      const uint8_t *payload, uint8_t len);

    // Hooks für spezialisierte Decoder
    virtual void onConfigLoaded() {}
    virtual void onOtaFinished() {}
    virtual void onFeatureFrame(uint8_t featureId,
                                uint8_t commandId,
                                const uint8_t *payload,
                                uint8_t payloadLen)
    {
        // Default: nichts tun
    }

    virtual uint8_t getAliveFeatureData(uint8_t *buf)
    {
        return 0; // Default: keine Zusatzdaten
    }

    virtual AliveState getStatus() const { return AliveState::Offline; }

protected:
    virtual void featureSetup() {}
    virtual void featureLoop() {}
};
