#pragma once
#include "Arduino.h"
#include "../core/DecoderBase.h"
#include <Wire.h>
#include "protocol_constants_core.h"

class PowerDecoder : public DecoderBase
{
public:

    void setup() override;
    void loop() override;

// Diese Methode existiert in DecoderBase und wird korrekt überschrieben
void onFeatureFrame(uint8_t featureId,
                    uint8_t commandId,
                    const uint8_t* payload,
                    uint8_t payloadLen) override;
    uint8_t getAliveFeatureData(uint8_t *buf) override;
    AliveState getStatus() const override;

private:
    void readCurrent();
    void shutdown(uint8_t channel);

    uint16_t current_mA[2] = {0, 0};

    static const uint8_t SDA_PIN = 6;
    static const uint8_t SCL_PIN = 7;

    bool shutdownActive[2] = {false, false};

    unsigned long lastSendTime = 0;
};

extern void loadDefaultsWithAddress();              // setzt seine Standardwerte und behält die Adresse bei

// im PowerDecoder
constexpr const char* DECODER_NVS_NAMESPACE = "power";
