#include "protocol_constants_core.h"
#if DECODER_TYPE == DEVTYPE_RM

#include <Arduino.h>
#include <esp_now.h>
#include <Ticker.h>
#include "soc/rtc_cntl_reg.h"
#include "FeedbackDecoder.h"
#include "CANguruPreferences.h"
#include "Core_EspNow.h"
#include "Core_Alive.h"
#include "Core_globals.h"
#include "DecoderBase.h"

uint8_t decoderadr;

// ---------------------------------------------------------------------------
// Hardware-Pins
// ---------------------------------------------------------------------------
const uint8_t zPin = GPIO_NUM_4;
const uint8_t s3Pin = GPIO_NUM_16;
const uint8_t s2Pin = GPIO_NUM_17;
const uint8_t s1Pin = GPIO_NUM_5;
const uint8_t s0Pin = GPIO_NUM_18;
const uint8_t enablePin = GPIO_NUM_19;

uint8_t pins[] = {enablePin, s0Pin, s1Pin, s2Pin, s3Pin};

// ---------------------------------------------------------------------------
// Kanal-Status
// ---------------------------------------------------------------------------
const uint8_t maxCntChannels = 16;
const uint8_t isFree = 0;
const uint8_t isOccupied = 1;

uint8_t channelStatus[maxCntChannels];
uint8_t msecs[maxCntChannels];
uint8_t inputValue[maxCntChannels];

boolean wdpAlive = false;

// ---------------------------------------------------------------------------
// Timer (NEUE stabile Version für ESP32 Arduino Core 3.x)
// ---------------------------------------------------------------------------
Ticker tckr0;
// 1 ms Timer – NICHT mehr abhängig von Kanalzahl
const float tckr0Time = 0.001f;
DecoderBase::AliveState currentStatus = DecoderBase::AliveState::Online;

static const char *feedBack_getLine0()
{
    return "CANguru-";
}

static const char *feedBack_getLine1()
{
    return "FeedBack ";
}

void FeedbackDecoderAfterStart()
{
    // Log optional
}

void FeedbackDecoderBeforeRestart()
{
    // Log optional
}

void initFeedbackDecoder()
{
    ConfigProvider_t provider = {
        .getLine0 = feedBack_getLine0,
        .getLine1 = feedBack_getLine1};

    registerConfigProvider(&provider);
}

void setFeedbackChannel(uint8_t channel, uint8_t status)
{
    if (channel < 1 || channel > maxCntChannels)
        return;

    char key[8];
    sprintf(key, key_channel, channel);
    prefs.putByte(key, status);
}

void saveFeedbackSettings(const uint8_t *values)
{
    for (int i = 0; i < maxCntChannels; i++)
    {
        setFeedbackChannel(i + 1, values[i]);
    }
}

// ---------------------------------------------------------------------------
// Feedback-Kanal aus Preferences
// ---------------------------------------------------------------------------
uint8_t getFeedbackChannel(uint8_t channel)
{
    char key[8];
    sprintf(key, key_channel, channel);
    return prefs.getByte(key, channel); // Default = Kanalnummer
}
void loadFeedbackSettings()
{

    for (int i = 0; i < maxCntChannels; i++)
    {
        char key[8];
        sprintf(key, key_channel, i + 1);

        uint8_t val = prefs.getByte(key, 0xFF);

        if (val == 0xFF) // Wert existiert nicht → erster Start
        {
            // Wenn erster Start → Defaults speichern
            setFeedbackChannel(i + 1, i + 1);
        }
        else
        {
            setFeedbackChannel(i + 1, val);
        }
    }
}

void sendFeedbackSettingsToBridge()
{
    uint8_t pkt[maxCntChannels];

    for (int i = 0; i < maxCntChannels; i++)
        pkt[i] = getFeedbackChannel(i + 1);

    gDecoder->sendToBridge(FEATURE_FEEDBACK, FEEDBACK_CMD_GET_SETTINGS, pkt, maxCntChannels);
}

void sendFeedbackSetAck()
{
    gDecoder->sendToBridge(FEATURE_FEEDBACK, FEEDBACK_CMD_SET_SETTINGS_ACK, nullptr, 0);
}
void loadDefaults(bool keepAdr) // setzt seine Standardwerte
{
    // Hier können die Standardwerte für den FeedbackDecoder gesetzt werden
    Serial.println("Loading default settings for FeedbackDecoder...");
    prefsFeedBack.init();
    //--------------------------------------------------- key -------------- Wert ------------min ----------max ---)
    // Adresse
    if (keepAdr)
        prefsFeedBack.putByte(key_Adr, decoderadr);
    else
        decoderadr = 1;
    decoderadr = readValfromPreferences8(prefsFeedBack, key_Adr, decoderadr, minadr, maxadr);
    loadFeedbackSettings();
}

void loadDefaultsWithAddress() // setzt seine Standardwerte und behält die Adresse bei
{
    loadDefaults(true);
}

// ---------------------------------------------------------------------------
// Ereignis an Bridge melden
// ---------------------------------------------------------------------------
void process_sensor_event(uint8_t channel)
{
    uint8_t opFrame[13] = {0};
    opFrame[CANcmd] = S88_EVENT_R;
    opFrame[hash0] = hasharr[0];
    opFrame[hash1] = hasharr[1];
    opFrame[Framelng] = 8;

    // Kontaktkennung
    opFrame[data3] = getFeedbackChannel(channel + 1);

    // Zustand alt / neu
    if (channelStatus[channel] == isOccupied)
    {
        opFrame[data4] = isFree;
        opFrame[data5] = isOccupied;
    }
    else
    {
        opFrame[data4] = isOccupied;
        opFrame[data5] = isFree;
    }

    coreEspNowSendCAN2Bridge(opFrame);
}

// ---------------------------------------------------------------------------
// Multiplexer lesen
// ---------------------------------------------------------------------------
int readPin(uint8_t inputPin)
{
    for (uint8_t bits = 0; bits < 4; bits++)
        digitalWrite(pins[bits + 1], bitRead(inputPin, bits));

    delayMicroseconds(1);
    return digitalRead(zPin);
}

// ---------------------------------------------------------------------------
// Timer-Callback (NEUE watchdog-sichere Version)
// ---------------------------------------------------------------------------
void IRAM_ATTR timer1ms()
{
    static uint8_t currChannel = 0;

    // Kanal lesen
    if (readPin(currChannel))
        inputValue[currChannel]++;

    msecs[currChannel]++;

    // Auswertung alle 50 ms
    if (msecs[currChannel] > 50)
    {
        bool occupied = (inputValue[currChannel] < 45) && (inputValue[currChannel] > 0);

        if (occupied && channelStatus[currChannel] != isOccupied)
        {
            channelStatus[currChannel] = isOccupied;
            process_sensor_event(currChannel);
        }
        else if (!occupied && channelStatus[currChannel] != isFree)
        {
            channelStatus[currChannel] = isFree;
            process_sensor_event(currChannel);
        }

        msecs[currChannel] = 0;
        inputValue[currChannel] = 0;
    }

    currChannel++;
    if (currChannel >= maxCntChannels)
        currChannel = 0;
}

// ---------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------
void FeedbackDecoder::setup()
{
    DecoderBase::setup();
    initFeedbackDecoder();
    beforeRestartCallback = FeedbackDecoderBeforeRestart;
    afterstartCallback = FeedbackDecoderAfterStart;
    DEVTYPE = DEVTYPE_RM;
    VERS_HIGH = 1;
    VERS_LOW = 0;
    wdpAlive = false;
    memset(msecs, 0, sizeof(msecs));
    memset(inputValue, 0, sizeof(inputValue));
    memset(channelStatus, isFree, sizeof(channelStatus));
    // nach dem ersten Mal Einlesen der gespeicherten Werte
    loadDefaults(false);
    // initialize digital pins as an input / output.
    pinMode(zPin, INPUT);
    for (uint8_t p = 0; p < sizeof(pins); p++)
        pinMode(pins[p], OUTPUT);
    digitalWrite(enablePin, LOW);
    // WICHTIG: Timer hier starten – NICHT im loop(), NICHT in onFeatureFrame()
    tckr0.attach(tckr0Time, timer1ms);
}

// ---------------------------------------------------------------------------
// Feature-Frames
// ---------------------------------------------------------------------------
void FeedbackDecoder::onFeatureFrame(uint8_t featureId,
                                     uint8_t commandId,
                                     const uint8_t *payload,
                                     uint8_t payloadLen)
{
    if (featureId == FEATURE_BRIDGE2DECODER)
    {
        switch (commandId)
        {
        case S88_EVENT:
        {
            // DLC = 4: Abfrage des aktuellen Status eines Eingangs, Anwort mit DLC = 8
            if (payload[Framelng] == 4)
            {
                if (!wdpAlive)
                {
                    wdpAlive = true;
                }
                // DLC = 4: Abfrage des aktuellen Status eines Eingangs, Antwort mit DLC = 8
                // D0/1 Gerätekenner
                // D2/3 Kontaktkennung
                Serial.printf("S88_EVENT: Gerätekenner %02X%02X, Kontaktkennung %02X\r\n", payload[hash0], payload[hash1], payload[data3]);
                for (uint8_t ch = 0; ch < maxCntChannels; ch++)
                    // Kanal mit dieser Kontaktkennung data3 suchen
                    if (getFeedbackChannel(ch + 1) == payload[data3])
                    {
                        Serial.printf("S88_EVENT: Kanal %d gefunden für Kontaktkennung %02X\r\n", ch + 1, payload[data3]);
                        // wenn gefunden, dann Meldung machen
                        process_sensor_event(ch);
                        break;
                    }
            }
        }
        }
    }

    if (featureId != FEATURE_BRIDGE2DECODER && featureId != FEATURE_FEEDBACK)
        return;

    if (featureId == FEATURE_FEEDBACK)
    {
        switch (commandId)
        {
        case FEEDBACK_CMD_GET_SETTINGS:
            sendFeedbackSettingsToBridge();
            break;

        case FEEDBACK_CMD_SET_SETTINGS:
            if (payloadLen == 17)
            {
                saveFeedbackSettings(&payload[1]);
                sendFeedbackSetAck();
            }
            break;
        }
    }
}

// ---------------------------------------------------------------------------
// Alive-Daten
// ---------------------------------------------------------------------------
uint8_t FeedbackDecoder::getAliveFeatureData(uint8_t *buf)
{
    buf[0] = 0x00;
    buf[1] = 0x00;
    buf[2] = 0x00;
    buf[3] = 0x00;
    return 4;
}

DecoderBase::AliveState FeedbackDecoder::getStatus() const
{
    return currentStatus; // Implement your status logic here
}

// ---------------------------------------------------------------------------
// Loop
// ---------------------------------------------------------------------------
void FeedbackDecoder::loop()
{
    DecoderBase::loop();
    // KEIN delay(10) mehr – Watchdog-sicher
}

#endif
