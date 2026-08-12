
#include "protocol_constants_core.h"
#if DECODER_TYPE == DEVTYPE_SIGNAL

#pragma once

#include "Arduino.h"
#include "core_globals.h"
#include "CANguruPreferences.h"

/////////////
// State‑Machine‑Variablen
extern int stepsToEnd;           // Anzahl Steps von Nullpunkt zum Endpunkt (wird in der Kalibrierung ermittelt)
extern uint16_t step_delay; 
enum stepDirections
{
    A_dir = 0,
    B_dir
};

// mögliche Positionen des steppers
enum position
{
    nullpkt = 0, // Standort vom Motor weg
    endpkt,    // Standort zum Motor hin
    any_pos
};

enum setupPhases
{
    phase0 = 0, // Idle
    phase1,     // SlowMove (Nullpunkt-Fahrt)
    phase2,     // ZeroPoint gesetzt
    phase3,     // Endpunkt-Fahrt
    phase4,     // Endpunkt gesetzt
    phase5      // Testlauf
};

// Verzögerungen

// Keys für NVS
static const char *key_Adr = "decAdr";
static const char *key_SrvDel = "swDelay";
static const char *key_stepsToEnd = "stepsToEnd";
static const char *key_curr_pos = "acc_state";
static const char *key_s_d = "s_d";

extern CANguruPreferences prefsSignal;
extern int stepsToEnd;
extern position current_position;
extern position current_destination;
extern stepDirections stepDirection;


//////////////

void Attach();
void SetDirection();
void StartSlowMove2Start(position toForward);
void SetZeroPoint();
void StartEndPointMove(position toForward);
void SetEndPoint();
void Start2Run(position dir);
void StepperTick();
void StopAll();
bool PosChg(position newPos);

#endif // DECODER_TYPE == DEVTYPE_SIGNAL
