#include "protocol_constants_core.h"
#if DECODER_TYPE == DEVTYPE_SWITCH

#include <Arduino.h>
#include "SwitchDecoder.h"
#include "CANguruPreferences.h"
#include "Core_EspNow.h"
#include "Core_Alive.h"
#include "core_globals.h"
#include "Core_Config.h"
#include "DecoderBase.h"
#include "Stepper_switch.h"

/*
Variablen der steppers & Magnetartikel
*/

uint8_t decoderadr;

unsigned long lastSendTime = 0;
DecoderBase::AliveState currentStatus = DecoderBase::AliveState::Online;

static const char *switch_getLine0()
{
    return "CANguru-";
}

static const char *switch_getLine1()
{
    return "Switch";
}

void SwitchDecoderAfterStart()
{
    // Log optional
}

void SwitchDecoderBeforeRestart()
{
    // Log optional
}

void initSwitchDecoder()
{
    ConfigProvider_t provider = {
        .getLine0 = switch_getLine0,
        .getLine1 = switch_getLine1};

    registerConfigProvider(&provider);
}

void loadDefaults(bool keepAdr) // setzt seine Standardwerte
{
    // Hier können die Standardwerte für den SwitchDecoder gesetzt werden
    Serial.println("Loading default settings for SwitchDecoder...");
    prefsSwitch.init(); // Muss hier aufgerufen werden, damit die Werte aus dem Preferences-Speicher gelesen werden können
    //--------------------------------------------------- key -------------- Wert ------------min ----------max ---)
    // Adresse
    // decoderadr ist nicht die ID des Decoders sondern die Adresse der Weiche, die dieser Decoder steuert.
    // Sie wird in den Einstellungen der GUI eingegeben und muss mit der tatsächlichen Adresse der Weiche übereinstimmen,
    // damit die Steuerung funktioniert.
    if (keepAdr)
        prefsSwitch.putByte(key_Adr, decoderadr);
    else
        decoderadr = 1;
    decoderadr = readValfromPreferences8(prefsSwitch, key_Adr, decoderadr, 1, 0xFF);
    // Verzögerung
    step_delay = step_delay_std;
    step_delay = readValfromPreferences16(prefsSwitch, key_SrvDel, step_delay_std, step_delay_min, step_delay_max);
    // Gesamtumdrehung
    stepsToEnd = stepsToEnd_std;
    stepsToEnd = readValfromPreferences16(prefsSwitch, key_stepsToEnd, stepsToEnd_std, stepsToEnd_min, stepsToEnd_max);
    // Status der Magnetartikel versenden an die steppers
    current_position = nullpkt; // Default-Position ist nullpkt, damit die GUI die Weiche in der Anfangsstellung anzeigt, wenn noch kein Testlauf durchgeführt wurde
    current_position = (position)readValfromPreferences8(prefsSwitch, key_curr_pos, nullpkt, nullpkt, endpkt);
    // Ausrichtung des Stepper Motors
    stepDirection = A_dir;
    stepDirection = (stepDirections)readValfromPreferences8(prefsSwitch, key_s_d, A_dir, A_dir, B_dir);
}

void loadDefaultsWithAddress() // setzt seine Standardwerte und behält die Adresse bei
{
    loadDefaults(true);
}

void SwitchDecoder::setup()
{
    // 0. Basisklasse initialisieren (WICHTIG!)
    DecoderBase::setup();
    beforeRestartCallback = SwitchDecoderBeforeRestart;
    afterstartCallback = SwitchDecoderAfterStart;
    DEVTYPE = DEVTYPE_SWITCH;
    VERS_HIGH = 1;
    VERS_LOW = 0;
    initSwitchDecoder();
    // nach dem ersten Mal Einlesen der gespeicherten Werte
    loadDefaults(false);
    Attach();
}

// Routine meldet an die CANguru-Bridge, wenn eine Statusänderung
// einer Weiche/Signal eingetreten ist
void acc_report()
{
    uint8_t opFrame[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    opFrame[CANcmd] = SWITCH_ACC_R;
    opFrame[hash0] = hasharr[0];
    opFrame[hash1] = hasharr[1];
    opFrame[Framelng] = 0x05;
    // Weichenadresse
    decoderadr = readValfromPreferences8(prefsSwitch, key_Adr, decoderadr, 1, 0xFF);
    int address = (uint16_t)(decoderadr >> 8) | decoderadr + MM_ACC - 1;
    opFrame[data2] = (uint8_t)(address >> 8);
    opFrame[data3] = (uint8_t)address;
    // Meldung der Lage
    opFrame[data4] = (uint8_t)readValfromPreferences8(prefsSwitch, key_curr_pos, position::nullpkt, position::nullpkt, position::endpkt);
    coreEspNowSendCAN2Bridge(opFrame);
    //%
}

void SwitchDecoder::onFeatureFrame(uint8_t featureId,
                                   uint8_t commandId,
                                   const uint8_t *payload,
                                   uint8_t payloadLen)
{
    const uint8_t payloadLenExpected = 8;
    if (featureId != FEATURE_brigde2Decoder && featureId != FEATURE_SWITCH)
        return;
    // Hier können eingehende Daten verarbeitet werden, z.B. ACKs oder Statusupdates
    if (featureId == FEATURE_brigde2Decoder)
    {
        switch (commandId)
        {
        case SWITCH_ACC:
        {
            uint8_t _to_address = (uint16_t)((payload[data2] << 8) | payload[data3]) - MM_ACC + 1;
            // Umsetzung nur bei gültiger Weichenadresse
            // Auf benutzte Adresse überprüfen
            if (_to_address == decoderadr)
            {
                position dir = any_pos;
                // 1 - gerade       endpkt  nahe am Motor
                // 0 - abzweigend   nullpkt   weit weg vom Motor
                //  muss Artikel geändert werden?
                if (PosChg(dir))
                {
                    Start2Run((position)payload[data4]);
                    acc_report();
                }
                break;
            }
            break;
        }
        }
    }
    if (featureId == FEATURE_SWITCH)
    {
        // Globale Mindestlänge prüfen
        if (payloadLen < 1)
        {
            Serial.println("ERROR: SWITCH frame without payload");
            return;
        }

        if (payload[0] != aliveGetDecoderID())
        {
            Serial.printf("ERROR: GET_SETTINGS for assignedId %d does not match this decoder's ID %d\r\n",
                          payload[0], aliveGetDecoderID());
            return;
        }

        switch (commandId)
        {
        case SWITCH_CMD_SET_SETTINGS:
            // Empfangen der Einstellungen von der GUI, z.B. nach Änderung in den Eingabefeldern und Klick auf "Senden"
            if (payloadLen < EXP_LNG_SWITCH_SETTINGS)
            {
                Serial.printf("ERROR: SET_SETTINGS payload too short (%d)\r\n", payloadLen);
                return;
            }
            // Decoderadresse
            prefsSwitch.putByte(key_Adr, payload[1]);
            decoderadr = readValfromPreferences8(prefsSwitch, key_Adr, decoderadr, 1, 0xFF);

            // Verzögerung
            prefsSwitch.putUInt(key_SrvDel, (payload[2] << 8) | payload[3]);
            step_delay = readValfromPreferences16(prefsSwitch, key_SrvDel, step_delay_std, step_delay_min, step_delay_max);
            // Gesamtumdrehung
            prefsSwitch.putUInt(key_stepsToEnd, (payload[4] << 8) | payload[5]);
            stepsToEnd = readValfromPreferences16(prefsSwitch, key_stepsToEnd, stepsToEnd_std, stepsToEnd_min, stepsToEnd_max);

            // Position
            prefsSwitch.putByte(key_curr_pos, (uint8_t)payload[6]);
            current_position = (position)readValfromPreferences8(prefsSwitch, key_curr_pos, position::nullpkt, position::nullpkt, position::endpkt);

            // Stepper‑Richtung
            prefsSwitch.putByte(key_s_d, payload[7]);
            stepDirection = (stepDirections)readValfromPreferences8(prefsSwitch, key_s_d, A_dir, A_dir, B_dir);

            SetDirection();
            break;

        case SWITCH_CMD_GET_SETTINGS:
            // GET_SETTINGS braucht genau 1 Byte Payload (assignedId)
            // sendet die aktuellen Einstellungen zurück an die GUI, damit die GUI die Werte in den Eingabefeldern korrekt anzeigen kann
            if (payloadLen < 1)
            {
                Serial.println("ERROR: GET_SETTINGS without assignedId");
                return;
            }

            uint8_t p[payloadLenExpected];
            p[0] = aliveGetDecoderID();

            decoderadr = readValfromPreferences8(prefsSwitch, key_Adr, decoderadr, 1, 0xFF);
            p[1] = decoderadr;

            step_delay = readValfromPreferences16(prefsSwitch, key_SrvDel, step_delay_std, step_delay_min, step_delay_max);
            p[2] = step_delay >> 8;
            p[3] = step_delay & 0xFF;

            stepsToEnd = readValfromPreferences16(prefsSwitch, key_stepsToEnd, stepsToEnd_std, stepsToEnd_min, stepsToEnd_max);
            p[4] = stepsToEnd >> 8;
            p[5] = stepsToEnd & 0xFF;

            p[6] = (uint8_t)readValfromPreferences8(prefsSwitch, key_curr_pos, position::nullpkt, position::nullpkt, position::endpkt);

            stepDirection = (stepDirections)readValfromPreferences8(prefsSwitch, key_s_d, A_dir, A_dir, B_dir);
            p[7] = stepDirection;

            gDecoder->sendToBridge(featureId, SWITCH_CMD_GET_SETTINGS, p, payloadLenExpected);
            break;

        case SWITCH_CMD_MOVE2START:
            if (payloadLen < 2)
            {
                Serial.println("ERROR: MOVE2START payload too short");
                return;
            }
            StartSlowMove2Start((position)payload[1]);
            break;

        case SWITCH_CMD_SET_ZERO:
            // kein payload[1] nötig
            SetZeroPoint();
            break;

        case SWITCH_CMD_MOVE2END:
            if (payloadLen < 2)
            {
                Serial.println("ERROR: MOVE2END payload too short");
                return;
            }
            StartEndPointMove((position)payload[1]);
            break;

        case SWITCH_CMD_SET_END:
            SetEndPoint();
            break;

        case SWITCH_CMD_TEST_RUN:
            if (payloadLen < 2)
            {
                Serial.println("ERROR: TEST_RUN payload too short");
                return;
            }
            Start2Run((position)payload[1]);
            break;

        case SWITCH_CMD_STOP_ALL:
            Serial.println("SwitchDecoder: STOP_ALL empfangen.");
            StopAll();
            break;
        }
    }
}

uint8_t SwitchDecoder::getAliveFeatureData(uint8_t *buf)
{
    buf[0] = 0x00;
    buf[1] = 0x00;
    buf[2] = 0x00;
    buf[3] = 0x00;

    return 4; // zwei Bytes Feature-Daten
}

DecoderBase::AliveState SwitchDecoder::getStatus() const
{
    return currentStatus; // Implement your status logic here
}

void SwitchDecoder::loop()
{
    // 0. das Erben von DecoderBase::loop() aufrufen, damit Core_Alive und Core_Ota funktionieren
    DecoderBase::loop();
    StepperTick();
    // Sehr kurze Pause, damit der Watchdog glücklich bleibt
    delay(5);
}

#endif