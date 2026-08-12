#include "protocol_constants_core.h"
#if DECODER_TYPE == DEVTYPE_POWER

#include <Arduino.h>
#include <esp_now.h>
#include "PowerDecoder.h"
#include "Adafruit_INA3221.h"
#include "CANguruPreferences.h"
#include "Core_EspNow.h"
#include "Core_Alive.h"
#include "Core_globals.h"
#include "DecoderBase.h"

uint16_t threshold_mA = 1000;

unsigned long lastSendTime = 0;
DecoderBase::AliveState currentStatus = DecoderBase::AliveState::Online;

// Globale Objekte
static Adafruit_INA3221 ina3221;
static const gpio_num_t shutdownPin = GPIO_NUM_20; // Define the GPIO pin for shutdown

// Keys für NVS
static const char *key_power_threshold = "pwrThreshold";

static const char *power_getLine0()
{
    return "CANguru-";
}

static const char *power_getLine1()
{
    return "Booster";
}

// Konfig lesen
int coreConfigGetUInt16(const char *key, int defaultValue)
{
    return prefsPower.getUInt(key, defaultValue);
}

// Konfig schreiben
void coreConfigSetUInt16(const char *key, int value)
{
    prefsPower.putUInt(key, value);
}

void setTrackPower(bool state)
{
    if (state)
    {
        Serial.println("POWER_CHG_TRACK_CURRENT EIN received");
        gpio_set_level(shutdownPin, HIGH); // Strom EIN
    }
    else
    {
        Serial.println("POWER_CHG_TRACK_CURRENT AUS received");
        gpio_set_level(shutdownPin, LOW); // Strom AUS
    }
}

void PowerDecoderAfterStart()
{
    // Abschalt-Pin konfigurieren
    gpio_set_direction(shutdownPin, GPIO_MODE_OUTPUT);

    // Gleisspannung ausschalten
    setTrackPower(false);

    // Log optional
}

void PowerDecoderBeforeRestart()
{
    // Gleisspannung abschalten
    setTrackPower(false);

    // INA3221 abschalten
    ina3221.reset();

    // Log optional
}

void initPowerDecoder()
{
    ConfigProvider_t provider = {
        .getLine0 = power_getLine0,
        .getLine1 = power_getLine1};

    registerConfigProvider(&provider);
}
void loadDefaults() // setzt seine Standardwerte
{
    // Hier können die Standardwerte für den PowerDecoder gesetzt werden
    Serial.println("Loading default settings for PowerDecoder...");
    //--------------------------------------------------- key -------------- Wert ------------min ----------max ---)
    // Adresse
    prefsPower.init();
    threshold_mA = coreConfigGetUInt16(key_power_threshold, 1000);
}

void loadDefaultsWithAddress() // setzt seine Standardwerte und behält die Adresse bei
{
    loadDefaults();
}

void PowerDecoder::setup()
{
    // 0. Basisklasse initialisieren (WICHTIG!)
    DecoderBase::setup();

    // 1. INA3221 initialisieren
    Wire.begin(SDA_PIN, SCL_PIN, 400000);
    if (!ina3221.begin(0x40, &Wire))
    {
        while (true)
        {
            Serial.println("INA3221 begin failed");
            delay(1000);
        }
    }
    // Grenzwert aus NVS laden
    beforeRestartCallback = PowerDecoderBeforeRestart;
    afterstartCallback = PowerDecoderAfterStart;
    DEVTYPE = DEVTYPE_POWER;
    VERS_HIGH = 1;
    VERS_LOW = 0;
    pinMode(shutdownPin, OUTPUT);

    initPowerDecoder();
    // NVS
    loadDefaults();
}

void PowerDecoder::readCurrent()
{
    static int addon = 0;
    for (uint8_t ch = 0; ch < 2; ch++)
    {
        float amps = ina3221.getCurrentAmps(ch);
        current_mA[ch] = (uint16_t)(amps * 1000.0f); // mA
    }
}

void PowerDecoder::shutdown(uint8_t channel)
{
    shutdownActive[channel] = true;
    currentStatus = DecoderBase::AliveState::Offline;
    setTrackPower(false);

    const uint8_t len = 3;
    uint8_t payload[len];
    payload[0] = channel;
    payload[1] = current_mA[channel] >> 8;
    payload[2] = current_mA[channel] & 0xFF;

    gDecoder->sendToBridge(FEATURE_POWER, POWER_CMD_SHUTDOWN, payload, len);
}

void PowerDecoder::onFeatureFrame(uint8_t featureId,
                                  uint8_t commandId,
                                  const uint8_t *payload,
                                  uint8_t payloadLen)
{
    if (featureId != FEATURE_POWER)
        return;

    switch (commandId)
    {
    case POWER_CMD_SET_THRESHOLD:
    {
        if (payloadLen < 2)
            return;

        threshold_mA = (payload[0] << 8) | payload[1];
        coreConfigSetUInt16(key_power_threshold, threshold_mA);

        Serial.printf("Threshold %u mA gespeichert\n", threshold_mA);
        break;
    }

    case POWER_CMD_GET_THRESHOLD:
    {
        uint16_t thr = coreConfigGetUInt16(key_power_threshold, 1350);

        const uint8_t len = 2;
        uint8_t resp[len] = {uint8_t(thr >> 8), uint8_t(thr & 0xFF)};

        gDecoder->sendToBridge(FEATURE_POWER, POWER_CMD_GET_THRESHOLD, resp, len);

        Serial.printf("Threshold %u mA gesendet\n", thr);
        break;
    }

    case POWER_CMD_SHUTDOWN:
    {
        // Shutdown kommt normalerweise vom Decoder selbst,
        // aber falls die Bridge es sendet:
        uint8_t channel = payload[0];
        shutdown(channel);
        break;
    }

    case POWER_CHG_TRACK_CURRENT:
    {
        // Shutdown kommt normalerweise vom Decoder selbst,
        // aber falls die Bridge es sendet:
        uint8_t state = payload[0];
        setTrackPower(state == 1);
        break;
    }

    case POWER_CMD_GET_VERSION:
    {

        const uint8_t len = 2;
        uint8_t resp[len] = {uint8_t(VERS_HIGH), uint8_t(VERS_LOW)};

        gDecoder->sendToBridge(FEATURE_POWER, POWER_CMD_GET_VERSION, resp, len);

        Serial.printf("Version %u.%u gesendet\n", VERS_HIGH, VERS_LOW);
        break;
    }
    }
}

uint8_t PowerDecoder::getAliveFeatureData(uint8_t *buf)
{
    uint16_t mA0 = current_mA[0]; // Assuming you want the first channel's current
    uint16_t mA1 = current_mA[1]; // Assuming you want the second channel's current

    buf[0] = mA0 >> 8;
    buf[1] = mA0 & 0xFF;
    buf[2] = mA1 >> 8;
    buf[3] = mA1 & 0xFF;

    return 4; // zwei Bytes Feature-Daten
}

DecoderBase::AliveState PowerDecoder::getStatus() const
{
    return currentStatus; // Implement your status logic here
}

void PowerDecoder::loop()
{
    // 0. das Erben von DecoderBase::loop() aufrufen, damit Core_Alive und Core_Ota funktionieren
    DecoderBase::loop();
    // 1. Messung immer durchführen
    if (aliveGetDecoderID() != 0)
        readCurrent();

    // 2. Shutdown sofort prüfen
    for (uint8_t ch = 0; ch < 2; ch++)
    {
        if (current_mA[ch] > threshold_mA && !shutdownActive[ch])
            shutdown(ch);
    }

    // 4. Sehr kurze Pause, damit der Watchdog glücklich bleibt
    delay(5);
}

#endif
