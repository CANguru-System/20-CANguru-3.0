
/* ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <CANguru-Buch@web.de> wrote this file. As long as you retain this
 * notice you can do whatever you want with this stuff. If we meet some day,
 * and you think this stuff is worth it, you can buy me a beer in return
 * Gustav Wostrack
 * ----------------------------------------------------------------------------
 */

#pragma once

#include "Arduino.h"
#include "OneButton.h"

#undef left2right

// mögliche Positionen des steppers
enum position
{
  right,
  left
};

/* Märklin Dokumentation:
Bit 0,1: Stellung:
00: Aus, Rund, Rot, Rechts, HP0
01: Ein, Grün, Gerade, HP1
10: Gelb, Links, HP2
11: Weiss, SH0
*/
//
enum enumway
{
  longway,  // von rechts nach links oder umgekehrt
  shortway, // der kleine Überschwinger beim Signal
  noway     // stepper steht
};

enum directions
{
  forward,
  reverse
};

enum setupPhases
{
  phase0,
  phase1,
  phase2,
  phase3,
  phase4
};

const int steps = 4; // how many pins are in use.

// MX1508 Motor Driver Pins
/*
// mit diesen Werten fährt der Stepper in die umgekehrte Richtung*/

#ifdef left2right
const uint8_t A_plus = GPIO_NUM_5;
const uint8_t A_minus = GPIO_NUM_6;
const uint8_t B_plus = GPIO_NUM_7;
const uint8_t B_minus = GPIO_NUM_10;
#else
const uint8_t A_plus = GPIO_NUM_5;
const uint8_t A_minus = GPIO_NUM_6;
const uint8_t B_plus = GPIO_NUM_7;
const uint8_t B_minus = GPIO_NUM_10;
#endif

const uint8_t btn_Step_pin = GPIO_NUM_21;
//const uint8_t btn_Step_pin = GPIO_NUM_9;

// Verzögerungen
const uint8_t maxstepperdelay = 10;
const uint8_t minstepperdelay = maxstepperdelay / 5;
const uint8_t stdstepperdelay = maxstepperdelay / 2;
const unsigned long step_delay_max = 1200;

void setContinue(bool c);
bool getContinue();

class StepperBase
{
public:
  // Voreinstellungen, steppernummer wird physikalisch mit
  // einem stepper verbunden;
  void Attach();
  // Setzt die Zielposition
  void SetPosition();
  // Zielposition ist links
  void GoLeft();
  // Zielposition ist rechts
  void GoRight();
  // Überprüft periodisch, ob die Zielposition erreicht wird
  void Update();
  // Setzt die Zielposition
  void SetPosDest(position p)
  {
    acc_pos_dest = p;
  }
  // Liefert die Zielposition
  position GetPosDest()
  {
    return acc_pos_dest;
  }
  // Setzt die aktuelle Position
  void SetPosCurr(position p)
  {
    acc_pos_curr = p;
  }
  // Liefert die aktuelle Position
  position GetPosCurr()
  {
    return acc_pos_curr;
  }
  // Hat sich die Position des steppers verändert?
  // Damit werden unnötige Positionsänderungen vermieden
  bool PosChg()
  {
    return acc_pos_dest != acc_pos_curr;
  }
  // Setzt den Wert für die Verzögerung
  void SetDelay(uint8_t delay_factor)
  {
    step_delay = step_delay_max * delay_factor; // 60L * 1000L / stepsPerRevolution / whatSpeed;
  }
  // Setzt die Adresse eines steppers
  void Set_to_address(uint8_t _to_address)
  {
    log_i("Set_to_address: %d", _to_address);
    acc__to_address = _to_address;
  }
  // Liefert die Adresse eines steppers
  uint8_t Get_to_address()
  {
    log_i("Get_to_address: %d", acc__to_address);
    return acc__to_address;
  }
  // Setzt die Gesamtumdrehungen eines steppers
  void Set_stepsToSwitch(uint16_t steps)
  {
    stepsToSwitch = steps;
  }
  // Liefert die Gesamtumdrehungen eines steppers
  uint16_t Get_stepsToSwitch()
  {
    return stepsToSwitch;
  }
  bool Get_set_stepsToSwitch()
  {
    return set_stepsToSwitch;
  }
  void Reset_stepsToSwitch()
  {
    set_stepsToSwitch = false;
  }
  bool is_no_Correction()
  {
    return no_correction;
  }

  void oneStep();
  void stopStepper();
  bool readyToStep;
  setupPhases phase;
  int8_t step;
  directions direction;
  unsigned long now_micros;
  unsigned long last_step_time;  // timestamp in us of when the last step was taken
  unsigned long step_delay;      // delay between steps, in us, based on speed
  unsigned long direction_delay; // delay between steps, in us, based on speed
  int16_t stepsToSwitch;
  int leftpos;   // 74 je groesser desto weiter nach links
  int rightpos;  // 5 je kleiner desto weiter nach rechts
  position acc_pos_curr;
  int currpos;   // current stepper position
  bool set_stepsToSwitch;
  int destpos;   // stepper position, where to go
  int increment; // increment to move for each interval

private:
  int endpos;
  uint8_t acc__to_address;
  position acc_pos_dest;
  int maxendpos; // * grdinmillis;
  enumway way;
  bool no_correction;
};

class StepperwButton : public StepperBase
{
private:
  OneButton button;
public:
  explicit StepperwButton(uint8_t pin):button(pin) {
  // setup interrupt routine
  // when not registering to the interrupt the sketch also works when the tick is called frequently.
    button.attachClick([](void *scope) { ((StepperwButton *) scope)->singleClick();}, this);
    button.attachDoubleClick([](void *scope) { ((StepperwButton *) scope)->doubleClick();}, this);
    button.attachLongPressStop([](void *scope) { ((StepperwButton *) scope)->longPressStop();}, this);
    button.attachMultiClick([](void *scope) { ((StepperwButton *) scope)->multiClick();}, this);
    button.setLongPressIntervalMs(1000);
  }

void runForward();
void runReverse();
void singleClick();
void doubleClick();
void longPressStop();
void multiClick();
void Update();

  void handle()
  {
    button.tick();
  }
};
