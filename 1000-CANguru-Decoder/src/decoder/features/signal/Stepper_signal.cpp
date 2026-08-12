
#include "protocol_constants_core.h"
#if DECODER_TYPE == DEVTYPE_SIGNAL

#include <Arduino.h>
#include <Stepper_signal.h>
#include "CANguruPreferences.h"
#include "Core_EspNow.h"
#include "DecoderBase.h"
/////////////////

setupPhases phase;
// State‑Machine‑Variablen
int runningsteps;             // aktuelle Position des Steppers (in Steps, Nullpunkt ist am Motor entfernten Punkt)
int stepsToEnd;               // Anzahl Steps von Nullpunkt zum Endpunkt (wird in der Kalibrierung ermittelt)
bool readyToStep;             // Flag, ob der Stepper gerade läuft (true) oder steht (false)
position current_position;    // aktuelle Position der Magnetartikel (nullpkt/endpkt)
position current_destination; // Zielposition der Magnetartikel (nullpkt/endpkt)
position direction;           // aktuelle Laufrichtung des Steppers
uint16_t step_delay;          // Verzögerung zwischen den Steps
int8_t step;                  // aktueller Step im Stepper-Zyklus
uint8_t A_plus;
uint8_t A_minus;
uint8_t B_plus;
uint8_t B_minus;
unsigned long last_step_time;
int destpos;
stepDirections stepDirection;

void SetDirection()
{
    switch (stepDirection)
    {
    case A_dir:
        log_d("Forward direction");
        A_plus = GPIO_NUM_10;
        A_minus = GPIO_NUM_7;
        B_plus = GPIO_NUM_6;
        B_minus = GPIO_NUM_5;
        break;
        // mit diesen Werten fährt der Stepper in die umgekehrte Richtung
    case B_dir:
        log_d("Reverse direction");
        A_plus = GPIO_NUM_5;
        A_minus = GPIO_NUM_6;
        B_plus = GPIO_NUM_7;
        B_minus = GPIO_NUM_10;
        break;
    }
}

void sendCurrPos2GUI(position curr_pos)
{
    // sendet die aktuelle Position der Magnetartikel an die GUI, damit die GUI die Weichenstellung korrekt anzeigen kann
    prefsSignal.putByte(key_curr_pos, curr_pos);
    current_position = curr_pos;
    const uint8_t len = 2;
    uint8_t p[len];
    p[0] = aliveGetDecoderID();
    p[1] = curr_pos; // RightOrLeft
    gDecoder->sendToBridge(FEATURE_SIGNAL, SIGNAL_CMD_SET_DIR, p, len);
}

// Schritt ausführen (MX1508)
void DoStep(position dir)
{
    switch (step)
    {
        // Bipolare Ansteuerung Vollschritt
        // 1a 1b 2a 2b
        // 1  0  0  1
        // 0  1  0  1
        // 0  1  1  0
        // 1  0  1  0

    case 0: // 1  0  0  1
        digitalWrite(A_plus, HIGH);
        digitalWrite(A_minus, LOW);
        digitalWrite(B_plus, LOW);
        digitalWrite(B_minus, HIGH);
        break;
    case 1: // 0  1  0  1
        digitalWrite(A_plus, LOW);
        digitalWrite(A_minus, HIGH);
        digitalWrite(B_plus, LOW);
        digitalWrite(B_minus, HIGH);
        break;
    case 2: // 0  1  1  0
        digitalWrite(A_plus, LOW);
        digitalWrite(A_minus, HIGH);
        digitalWrite(B_plus, HIGH);
        digitalWrite(B_minus, LOW);
        break;
    case 3: // 1  0  1  0
        digitalWrite(A_plus, HIGH);
        digitalWrite(A_minus, LOW);
        digitalWrite(B_plus, HIGH);
        digitalWrite(B_minus, LOW);
        break;
    }

    if (dir == position::nullpkt)
        step = (step + 1) % 4;
    else
        step = (step + 3) % 4;
}

int setcurrpos(position dir)
{
    if (direction == position::nullpkt)
        // läuft Richtung Nullpunkt, also weg vom Motor, dann wird die aktuelle Position um eins kleiner
        return runningsteps -= 1;
    else
        // läuft Richtung Endpunkt, also weg vom Motor, dann wird die aktuelle Position um eins größer
        return runningsteps += 1;
}

// Step‑Machine‑Tick (wird im Timer/Loop aufgerufen)
void StepperTick()
{
    if (!readyToStep)
        return;

    unsigned long now = micros();
    if (now - last_step_time < step_delay * 10)
        return;

    last_step_time = now;

    switch (phase)
    {
    case phase1: // Langsamfahrt zur Nullposition
        DoStep(direction);
        runningsteps = setcurrpos(direction);
        break;

    case phase3: // Endpunktfahrt starten
        DoStep(direction);
        runningsteps = setcurrpos(direction);
        break;

    case phase5: // Testlauf
        if ((runningsteps == destpos) || (runningsteps < 0))
        {
            readyToStep = false;
            if (runningsteps == stepsToEnd)
                sendCurrPos2GUI(position::endpkt);
            else
                sendCurrPos2GUI(position::nullpkt);
            phase = phase0;
        }
        else
        {
            runningsteps = setcurrpos(direction);
            DoStep(direction);
        }
        break;

    case phase0: // idle
    case phase2: // Nullpunkt setzen
    case phase4: // Endpunkt gesetzt
    default:
        // nichts tun
        break;
    }
}

void stopStepper()
{
    digitalWrite(A_plus, LOW);
    digitalWrite(A_minus, LOW);
    digitalWrite(B_plus, LOW);
    digitalWrite(B_minus, LOW);
}

// Langsamfahrt zur Nullposition
void StartSlowMove2Start(position dir)
{
    // SWITCH_CMD_SLOW_MOVE2START
    // der Stepper läuft langsam vom Motor weg.
    // keine Meldung an die GUI, da die GUI ja die Fahrt initiiert hat und damit weiß, dass sie läuft.
    direction = dir;             // links in der GUI ist null, rechts ist eins
    step_delay = step_delay_max; // langsam
    readyToStep = true;
    phase = phase1; // Freifahrt
}

// Nullpunkt setzen
void SetZeroPoint()
{
    // SWITCH_CMD_SET_ZERO
    // der Stepper wird am Ende des Laufweges, entfernt vom Stepper, gestoppt,
    // currpos ist jetzt Null.
    // keine Meldung an die GUI
    readyToStep = false;
    runningsteps = 0;
    sendCurrPos2GUI(position::nullpkt);
    phase = phase2;
}

// Endpunktfahrt starten
void StartEndPointMove(position dir)
{
    // SWITCH_CMD_MOVE2END
    // die Laufrichtung wird umgekehrt
    // der Stepper läuft langsam in Richtung Motor
    // keine Meldung an die GUI
    direction = dir; // links in der GUI ist null, rechts ist eins
    step_delay = step_delay_max;
    readyToStep = true;
    phase = phase3;
}

// Endpunkt setzen
void SetEndPoint()
{
    // SWITCH_CMD_SET_END
    // der Stepper wird am Anfang des Laufweges, nahe aam Stepper, gestoppt,
    // runningsteps ist jetzt Endpunkt.
    readyToStep = false;
    stepsToEnd = abs(runningsteps);
    prefsSignal.putUInt(key_stepsToEnd, stepsToEnd);
    const uint8_t len = 2;
    uint8_t p_ack[len] = {uint8_t(stepsToEnd >> 8), uint8_t(stepsToEnd & 0xFF)};
    gDecoder->sendToBridge(FEATURE_SIGNAL, SIGNAL_CMD_SET_END_ACK, p_ack, len);
    delay(100); // kurze Pause, damit die GUI die ACK-Nachricht verarbeiten kann, bevor die nächste Nachricht mit der neuen Position gesendet wird
    sendCurrPos2GUI(position::endpkt);
    step_delay = readValfromPreferences16(prefsSignal, key_SrvDel, step_delay_std, step_delay_min, step_delay_max);
    phase = phase4;
}

// Testfahrt
void Start2Run(position dir)
{
    // SWITCH_CMD_TEST_RUN
    current_destination = dir; // Zielposition der Magnetartikel (nullpkt/endpkt)
    // current position und Zielposition vergleichen, Richtung setzen, Fahrt starten
    current_position = (position)readValfromPreferences8(prefsSignal, key_curr_pos, position::nullpkt, position::nullpkt, position::endpkt);
    step_delay = readValfromPreferences16(prefsSignal, key_SrvDel, step_delay_std, step_delay_min, step_delay_max);
    if (current_position == current_destination)
    {
        Serial.println("SignalDecoder: Testlauf nicht möglich, da aktuelle Position bereits erreicht ist.");
        return;
    }
    if (current_position == position::nullpkt && dir == position::endpkt)
    {
        direction = position::endpkt; // Laufrichtung vom Motor weg
        // 1 - gerade       endpkt  nahe am Motor
        // zum Endpunkt fahren
        runningsteps = 0; // Nullpunkt ist am Motor entfernten Punkt
        destpos = stepsToEnd;
    }
    else if (current_position == position::endpkt && dir == position::nullpkt)
    { 
        direction = position::nullpkt; // Laufrichtung zum Motor hin
        // 0 - abzweigend   nullpkt   weit weg vom Motor
        // zum NullPunkt fahren
        runningsteps = stepsToEnd; // Endpunkt ist nahe am Motor
        destpos = 0;
    }

    phase = phase5;
    readyToStep = true;
}

void StopAll()
{
    // SWITCH_CMD_STOP_ALL
    // Alle laufenden Bewegungen stoppen
    readyToStep = false;
    stopStepper();
}
/////////////////

void Attach()
{
    // setup the pins on the microcontroller:
    SetDirection();
    phase = phase0;
    runningsteps = 0;
    stepsToEnd = 0;
    readyToStep = false;
    direction = any_pos;

    pinMode(A_plus, OUTPUT);
    pinMode(A_minus, OUTPUT);
    pinMode(B_plus, OUTPUT);
    pinMode(B_minus, OUTPUT);
    stopStepper();
}

bool PosChg(position newPos)
{
    current_destination = (position)newPos;
    return current_position != current_destination;
}

#endif // DECODER_TYPE == DEVTYPE_SWITCH
