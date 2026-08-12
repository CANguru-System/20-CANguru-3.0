#pragma once
#include "DecoderBase.h"

class SignalDecoder : public DecoderBase {
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

};
extern void loadDefaultsWithAddress();              // setzt seine Standardwerte und behält die Adresse bei
// z.B. im SignalDecoder
constexpr const char* DECODER_NVS_NAMESPACE = "signal";
