
/* ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <CANguru-Buch@web.de> wrote this file. As long as you retain this
 * notice you can do whatever you want with this stuff. If we meet some day,
 * and you think this stuff is worth it, you can buy me a beer in return
 * Gustav Wostrack
 * ----------------------------------------------------------------------------
 */

#include <Stepper.h>
#include "Preferences.h"

// ****** doubleclick
// das System befindet sich in der Anlaufphase (phase0), i.e. Neuinstallation oder Zurücksetzen des Decoders.
// startet die Prozedur, mit der der Laufweg des Steppers eingestellt wird.
// Nach dem doubleClick
// phase1:
// läuft der Stepper vom Motor weg. Ist der Stepper am Ende angekommen, wird
// ein singleClick
// phase2:
// gedrückt. Daraufhin läuft der Stepperarm in Richtung Motor. Ist er an diesem
// Ende angekommen, wird erneut
// ein singleClick
// phase3:
// gedrückt und der Motor läuft in die Anfangsposition auf der motorabgewandten Seite.
// dort angekommen beginnt erneut phase0 (Betriebsphase)

// ****** longClick
// das System befindet sich in der Anlaufphase (phase0), i.e. Neuinstallation oder Zurücksetzen des Decoders.
// startet eine Prozedur, in der der Stepper in die Mitte des Laufweges verschoben werden kann.
// Nach dem longClick (loslassen des Knopfes):
// phase4:
// A.: doubleCick:
// phase5:
// Stepper läuft in Richtung Motor, Nach einem
// singleClick:
// phase6: bleibt der Motor stehen, dann wieder phase0.
// B.: singleClick:
// phase6:
// Stepper läuft vom Motor weg. Nach einem
// singleClick:
// phase6: bleibt der Motor stehen, dann wieder phase0.

// Voreinstellungen, steppernummer wird physikalisch mit
// einem stepper verbunden

bool bcontinue;
const uint8_t add_delay = 3;

void setContinue(bool c)
{
  bcontinue = c;
}

bool getContinue()
{
  return bcontinue;
}

void StepperwButton::runReverse()
{
  // läuft in Richtung der zum Motor entgegengesetzten Ende
  direction = reverse;
  step = steps - 1;
  // run to one end until button is pressed
  // step one revolution in one direction:
  // move only if the appropriate delay has passed:
  last_step_time = 0;
  while (getContinue())
  {
    now_micros = micros();
    if ((now_micros - last_step_time) >= step_delay * add_delay)
    {
      // get the timeStamp of when you stepped:
      last_step_time = now_micros;
      oneStep();
    }
  }
} // runReverse

void StepperwButton::runForward()
{
  // läuft in Richtung des Motors
  direction = forward;
  step = 0;
  // run to the opposite end until button is pressed
  // step one revolution in one direction:
  last_step_time = 0;
  while (getContinue())
  {
    now_micros = micros();
    if ((now_micros - last_step_time) >= step_delay * add_delay)
    {
      // get the timeStamp of when you stepped:
      last_step_time = now_micros;
      oneStep();
    }
  }
} // runReverse

// this function will be called when the button was pressed a single time.
void StepperwButton::singleClick()
{
  log_d("singleClick");
  switch (phase)
  {
  case phase3:
    // der Stepper läuft erneut
    setContinue(true);
    switch (direction)
    {
    case forward:
      log_d("Forward: stepper runs again");
      runForward();
      break;
    case reverse:
      log_d("Reverse: stepper runs again");
      runReverse();
      break;
    }
    phase = phase2;
    break;
  case phase2:
    // der Stepper läuft und wird nun gestoppt
    log_d("Forward/Reverse: stepper stops");
    stopStepper();
    switch (direction)
    {
    case forward:
      log_d("Forward: turns to Reverse");
      direction = reverse;
      break;
    case reverse:
      log_d("Reverse: turns to forward");
      direction = forward;
      break;
    }
    phase = phase3;
    break;
  case phase1:
    // in die festgelegte Richtung wird gesteppt
    setContinue(true);
    switch (direction)
    {
    case forward:
      log_d("Forward: stepper starts to run");
      runForward();
      break;
    case reverse:
      log_d("Reverse: stepper starts to run");
      runReverse();
      break;
    }
    phase = phase2;
    break;
  case phase0:
    // die Richtung wird festgelegt
    log_d("Forward: set direction to motor end");
    direction = forward;
    phase = phase1;
    break;
  }
} // singleClick

// this function will be called when the button was pressed 2 times in a short timeframe.
void StepperwButton::doubleClick()
{
  log_d("doubleClick");
  if (phase == phase0)
  {
    // die Richtung wird festgelegt
    log_d("Reverse: set direction to non motor end");
    direction = reverse;
    phase = phase1;
  }
} // doubleClick

void StepperwButton::longPressStop()
{
  Upwardpos = stepsToSwitch;
  acc_pos_curr = Upward;
  phase = phase0;
  currpos = 0;
  set_stepsToSwitch = true;
  readyToStep = stepsToSwitch != 0;
  GoUpward();
} // longPressStop

void StepperwButton::multiClick()
{
  if (button.getNumberClicks() == 3)
  {
    log_d("multiClick 3");
    log_d("Reset direction");
    phase = phase0;
  }
  if (button.getNumberClicks() == 4)
  {
    log_d("multiClick 4");
    switch (stepDir)
    {
    case A_dir:
      SetDirection(B_dir);
      break;
    case B_dir:
      SetDirection(A_dir);
      break;
    }
  }
  if (button.getNumberClicks() == 5)
  {
    log_d("multiClick 5");
    log_d("Restart ESP32");
    ESP.restart();
  }
}

void StepperBase::Attach(stepDirections dir)
{
  // setup the pins on the microcontroller:
  if (pref.begin("CANguru", false))
  {
    log_d("Preferences erfolgreich gestartet");
  }
  SetDirection(dir);
  pinMode(A_plus, OUTPUT);
  pinMode(A_minus, OUTPUT);
  pinMode(B_plus, OUTPUT);
  pinMode(B_minus, OUTPUT);
  stopStepper();
  last_step_time = 0;
  direction_delay = 250; // 20 * step_delay/1000;
  phase = phase0;
  readyToStep = stepsToSwitch != 0;
  set_stepsToSwitch = false;
  no_correction = true;
  Downpos = 0;
  Upwardpos = stepsToSwitch;
  wippDist = stepsToSwitch / 4;
  switch (acc_pos_curr)
  {
  case Down:
    currpos = Downpos;
    break;
  case Upward:
    currpos = Upwardpos;
    break;
  }
}

void StepperBase::oneStep()
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
  switch (direction)
  {
  case forward:
    step++;
    if (step >= steps)
      step = 0;
    break;
  case reverse:
    step--;
    if (step < 0)
      step = steps - 1;
    break;
  }
}

void StepperBase::stopStepper()
{
  digitalWrite(A_plus, LOW);
  digitalWrite(A_minus, LOW);
  digitalWrite(B_plus, LOW);
  digitalWrite(B_minus, LOW);
}

// Setzt die Zielposition
void StepperBase::SetPosition()
{
  last_step_time = micros();
  acc_pos_dest = acc_pos_curr;
  switch (acc_pos_dest)
  {
  case Upward:
  {
    GoUpward();
  }
  break;
  case Down:
  {
    GoDown();
  }
  break;
  }
}

// Zielposition ist links/up
void StepperBase::_GoUpward(int dest)
{
  log_d("going Upward C: %d - d: %d", currpos, Upwardpos);
  destpos = dest;
  increment = 1;
  step = 0;
  direction = reverse;
}

void StepperBase::GoUpward()
{
  _GoUpward(Upwardpos);
  currWippPhase = hi0;
}

// Zielposition ist rechts/down
void StepperBase::_GoDown(int dest)
{
  log_d("going Down C: %d - d: %d", currpos, Upwardpos);
  destpos = dest;
  increment = -1;
  step = steps - 1;
  direction = forward;
}

// Zielposition ist rechts/down
void StepperBase::GoDown()
{
  _GoDown(Downpos);
  currWippPhase = lo0;
}

// Überprüft periodisch, ob die Zielposition erreicht wird
void StepperwButton::Update()
{
  handle();
  if (readyToStep && (destpos != currpos))
  {
    if (micros() - last_step_time >= step_delay)
    {
      // get the timeStamp of when you stepped:
      last_step_time = micros();
      oneStep();
      currpos += increment;
      if (destpos == currpos)
        switch (currWippPhase)
        {
        case hi0:
          currWippPhase = hi1;
          _GoDown(currpos - 3 * wippDist);
          break;
        case hi1:
          currWippPhase = hi2;
          _GoUpward(Upwardpos);
          break;
        case hi2:
          currWippPhase = hi3;
          _GoDown(currpos - 1.5 * wippDist);
          break;
        case hi3:
          currWippPhase = hi4;
          _GoUpward(Upwardpos);
          break;
        case hi4:
          stopStepper();
          break;
        case lo0:
          currWippPhase = lo1;
          _GoUpward(currpos + 2 * wippDist);
          break;
        case lo1:
          currWippPhase = lo2;
          _GoDown(Downpos);
          break;
        case lo2:
          currWippPhase = lo3;
          _GoUpward(currpos + wippDist);
          break;
        case lo3:
          currWippPhase = lo4;
          _GoDown(Downpos);
          break;
        case lo4:
          stopStepper();
          break;
        }
    }
  }
}
