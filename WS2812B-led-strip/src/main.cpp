#include <Arduino.h>
#define FASTLED_INTERNAL
#include <FastLED.h>

#define ONBOARD_LED 8

#define NUM_LEDS 24 // FastLED definitions
#define LED_PIN 2

#define TIMES_PER_SECOND(x) EVERY_N_MILLISECONDS(1000 / x)
#define ARRAYSIZE(x) (sizeof(x) / sizeof(x[0]))

CRGB g_LEDs[NUM_LEDS] = {0}; // Frame buffer for FastLED

int g_Brightness = 10;

#include "marquee.h"
#include "twinkle.h"
#include "comet.h"
#include "bounce.h"
#include "breathing.h"
#include "rgb.h"
#include "fill-grad.h"
#include "flicker.h"

//hw_timer_t *timer = NULL;
//portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

uint8_t effect = 0;
bool firstRun;

// FramesPerSecond
//
// Tracks a weighted average to smooth out the values that it calcs as the simple reciprocal
// of the amount of time taken specified by the caller.  So 1/3 of a second is 3 fps, and it
// will take up to 10 frames or so to stabilize on that value.
/*
bool bLED = 0;
uint8_t initialHue = 0;
const uint8_t deltaHue = 1;
const uint8_t hueDensity = 1;

void IRAM_ATTR onTimer()
{
  //  portENTER_CRITICAL_ISR(&timerMux);
  bLED = !bLED;
  BouncingBallEffect balls(NUM_LEDS, 1, 150);

  switch (effect)
  {
  case 0:
    DrawTwinkle();
    break;
  case 1:
    DrawComet();
    break;
  case 2:
    fill_rainbow(g_LEDs, NUM_LEDS, initialHue += hueDensity, deltaHue);
    FastLED.show();
    break;
  case 3:
    fill_solid(g_LEDs, NUM_LEDS, CRGB(15, 203, 255));
    FastLED.show();
    break;

  default:
    break;
  }
  effect++;
  if (effect > 3)
  {
    effect = 0;
  }
  Serial.println("Interrupt ausgelöst!");
  //  portEXIT_CRITICAL_ISR(&timerMux);
}
*/
void setup()
{
  Serial.begin(115200);
/*
  pinMode(ONBOARD_LED, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  FastLED.addLeds<WS2812B, LED_PIN, GRB>(g_LEDs, NUM_LEDS); // Add our LED strip to the FastLED library
  FastLED.setBrightness(120);

  FastLED.clear();
  */
  firstRun = true;
}

void loop()
{
  if (firstRun)
  {
    firstRun = false;
    Serial.println("LED Effects!");
    ///
    log_i("\r\n\r\nCANguru - WS2812b");
    log_i("\n on %s", ARDUINO_BOARD);
    log_i("CPU Frequency = %d Mhz", F_CPU / 1000000);
    //  log_e("ERROR!");
    //  log_d("VERBOSE");
    //  log_w("WARNING");
    //  log_d("INFO");
    //    DrawTwinkle();
    //    FastLED.show();

    // Timer 0, Divider 80 -> 1 Takt = 1 µs (bei 80 MHz APB Clock)
    /*  timer = timerBegin(0, 80000, true); // 80.000 = 1 ms, 5.000.000 = 5 s
      timerAttachInterrupt(timer, &onTimer, true);
      timerAlarmWrite(timer, 5000000, true); // 5.000.000 µs = 5 s
      timerAlarmEnable(timer);
      */
  }

  /*  while (true)
    {
      if (millis() - lastTime_house >= baseTime_house)
      {
        bLED = !bLED;
        switch (effect)
        {
        case 0:
          DrawTwinkle();
          break;
        case 1:
          DrawComet();
          break;
        case 2:
          fill_rainbow(g_LEDs, NUM_LEDS, initialHue += hueDensity, deltaHue);
          FastLED.show();
          break;
        case 3:
          fill_solid(g_LEDs, NUM_LEDS, CRGB(15, 203, 255));
          FastLED.show();
          break;

        default:
          break;
        }
      }
      effect++;
      if (effect > 3)
      {
        effect = 0;
      }*/
  // Blink the LED off and on
  // digitalWrite(ONBOARD_LED, bLED);

  /* Handle LEDs */

  /*
  fill_rainbow(g_LEDs, NUM_LEDS, initialHue += hueDensity, deltaHue);
  FastLED.show();
  */

  /*
  fill_solid(g_LEDs, NUM_LEDS, CRGB(15, 203, 255));
  FastLED.show();
  */

  /*
  for (int i = 0; i < NUM_LEDS; i++)
  g_LEDs[i] = CRGB::DarkOrange;
  */

  // DrawMarquee();

  // balls.Draw();

  // breathing();

  // rgb();

  // fill_gradient_HSV(g_LEDs, NUM_LEDS, CHSV(0,255,255), CHSV(60,255,255), SHORTEST_HUES);

  // fill_grad();

  // flicker();

  // flicker_random();
  //  }
}