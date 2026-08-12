#pragma once

#include <Arduino.h>
#include "DecoderBase.h"

// Der FeedbackDecoder ist ein CANguru-Decoder für S88-Rückmeldungen.
// Er nutzt Multiplexing über die Pins s0..s3 und zPin, um 16 Kanäle auszulesen.

class FeedbackDecoder : public DecoderBase
{
public:
    // -----------------------------------------------------------------------
    // Konstruktor
    // -----------------------------------------------------------------------
    FeedbackDecoder() = default;

    // -----------------------------------------------------------------------
    // Setup – wird von DecoderBase::setup() aufgerufen
    // -----------------------------------------------------------------------
    void setup() override;

    // -----------------------------------------------------------------------
    // Hauptschleife – ruft DecoderBase::loop() auf
    // -----------------------------------------------------------------------
    void loop() override;

    // -----------------------------------------------------------------------
    // Verarbeitung eingehender Feature-Frames
    // -----------------------------------------------------------------------
    void onFeatureFrame(uint8_t featureId,
                        uint8_t commandId,
                        const uint8_t *payload,
                        uint8_t payloadLen) override;

    // -----------------------------------------------------------------------
    // Alive-Daten für die Bridge
    // -----------------------------------------------------------------------
    uint8_t getAliveFeatureData(uint8_t *buf) override;

    // -----------------------------------------------------------------------
    // Status des Decoders (Alive / Offline / Error)
    // -----------------------------------------------------------------------
    DecoderBase::AliveState getStatus() const override;

private:
    // Alive-Status
    DecoderBase::AliveState currentStatus = DecoderBase::AliveState::Online;
};

extern void loadDefaultsWithAddress();              // setzt seine Standardwerte und behält die Adresse bei
// z.B. im FeedbackDecoder
constexpr const char* DECODER_NVS_NAMESPACE = "feedback";

// Keys für NVS
static const char *key_Adr = "decAdr";
static const char *key_channel = "K%d";
const uint8_t minadr = 1;
const uint8_t maxadr = 0xFF;

// im Servodecoder
//constexpr const char* DECODER_NVS_NAMESPACE = "servo";
