#include <Arduino.h>
#define FASTLED_INTERNAL
#include <FastLED.h>

#define ONBOARD_LED 8

#define NUM_LEDS 8*32 // FastLED definitions
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

#define TIMES_PER_SECOND(x) EVERY_N_MILLISECONDS(1000 / x)
#define ARRAYSIZE(x) (sizeof(x) / sizeof(x[0]))

hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
uint8_t effect = 0;
bool bLED = 0;
uint8_t initialHue = 0;
const uint8_t deltaHue = 1;
const uint8_t hueDensity = 1;

int16_t random(int MaxZahl)
{
  float zufall;
  zufall = (float)rand() / RAND_MAX * MaxZahl + 1;
  return ((int16_t)zufall);
}

// Funktion zur Erzeugung einer zufälligen Zeit (zwischen 500ms und 3000ms)
uint32_t getRandomTime() {
  return random(500000, 3000000); // Mikrosekunden
}

void IRAM_ATTR onTimer()
{
  portENTER_CRITICAL_ISR(&timerMux);
  bLED = !bLED;
  if (bLED)
    digitalWrite(ONBOARD_LED, HIGH);
  else
    digitalWrite(ONBOARD_LED, LOW);
  effect = random(11);
 // Neue zufällige Zeit setzen
  timerAlarmWrite(timer, getRandomTime(), true);
  portEXIT_CRITICAL_ISR(&timerMux);
}

void setup()
{
  pinMode(ONBOARD_LED, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  randomSeed(esp_random());

  FastLED.addLeds<WS2812B, LED_PIN, GRB>(g_LEDs, NUM_LEDS); // Add our LED strip to the FastLED library
  FastLED.setBrightness(120);

  FastLED.clear();
  Serial.begin(115200);
  ///
  // Timer 0, Divider 80 -> 1 Takt = 1 µs (bei 80 MHz APB Clock)
  timer = timerBegin(0, 80, true); // 80.000 = 1 ms, 5.000.000 = 5 s
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, getRandomTime(), true); // 5.000.000 µs = 5 s
  timerAlarmEnable(timer);
}

void loop()
{

  uint8_t initialHue = 0;
  const uint8_t deltaHue = 1;
  const uint8_t hueDensity = 1;

  BouncingBallEffect balls(NUM_LEDS, 1, 150);

  FastLED.setBrightness(g_Brightness);

  while (true)
  {
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
    case 4:
      for (int i = 0; i < NUM_LEDS; i++)
        g_LEDs[i] = CRGB::DarkOrange;
      break;
    case 5:
      DrawMarquee();
      break;
    case 6:
      balls.Draw();
      break;
    case 7:
      rgb();
      break;
    case 8:
      fill_gradient_HSV(g_LEDs, NUM_LEDS, CHSV(0, 255, 255), CHSV(60, 255, 255), SHORTEST_HUES);
      break;
    case 9:
      fill_grad();
      break;
    case 10:
      flicker();
      break;
    case 11:
      flicker_random();
      break;

    default:
      break;
    }
  }
}