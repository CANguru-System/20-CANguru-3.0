#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "effects.h"

extern Adafruit_NeoPixel strip;
extern bool ident;

void own_clear()
{
  for (int led = 0; led < LED_COUNT_NORM; led++)
  {
    strip.setPixelColor(led, strip.Color(0, 0, 0)); // Schwarz (aus)
    strip.show();
  }
}

void ownshow()
{
  strip.show();
  if (ident)
  {
    ident = false;
    for (uint8_t i = 0; i < 3; i++)
    {
      pulseWhite(1, 0, 32);
      own_clear();
      delay(500);
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
/**
 * The function `Wheel` takes a byte input `WheelPos` and returns a color value based on a rainbow
 * color wheel algorithm.
 *
 * @param WheelPos The `WheelPos` parameter is a byte value representing the position of a color wheel.
 * The function `Wheel` takes this position as input and calculates the corresponding RGB color value
 * based on the position on the color wheel.
 *
 * @return The `Wheel` function takes a `byte` parameter `WheelPos`, performs some calculations based
 * on its value, and returns an RGB color value using the `strip.Color` function. The specific color
 * returned depends on the value of `WheelPos` according to the conditional statements in the function.
 */
uint32_t Wheel(byte WheelPos)
{
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85)
  {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170)
  {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait)
{
  for (uint16_t i = 0; i < strip.numPixels(); i++)
  {
    strip.setPixelColor(i, c);
    ownshow();
    delay(wait);
  }
}

void rainbow(uint8_t wait)
{
  uint16_t i, j;

  for (j = 0; j < 256; j++)
  {
    for (i = 0; i < strip.numPixels(); i++)
    {
      strip.setPixelColor(i, Wheel((i + j) & 255));
    }
    ownshow();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait)
{
  uint16_t i, j;

  for (j = 0; j < 256 * 5; j++)
  { // 5 cycles of all colors on wheel
    for (i = 0; i < strip.numPixels(); i++)
    {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    ownshow();
    delay(wait);
  }
}

// Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait)
{
  for (int j = 0; j < 10; j++)
  { // do 10 cycles of chasing
    for (int q = 0; q < 3; q++)
    {
      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3)
      {
        strip.setPixelColor(i + q, c); // turn every third pixel on
      }
      ownshow();

      delay(wait);

      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3)
      {
        strip.setPixelColor(i + q, 0); // turn every third pixel off
      }
    }
  }
}

// Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait)
{
  for (int j = 0; j < 256; j++)
  { // cycle all 256 colors in the wheel
    for (int q = 0; q < 3; q++)
    {
      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3)
      {
        strip.setPixelColor(i + q, Wheel((i + j) % 255)); // turn every third pixel on
      }
      ownshow();

      delay(wait);

      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3)
      {
        strip.setPixelColor(i + q, 0); // turn every third pixel off
      }
    }
  }
}

// neu
void whiteOverRainbow(int whiteSpeed, int whiteLength)
{

  if (whiteLength >= strip.numPixels())
    whiteLength = strip.numPixels() - 1;

  int head = whiteLength - 1;
  int tail = 0;
  int loops = 3;
  int loopNum = 0;
  uint32_t lastTime = millis();
  uint32_t firstPixelHue = 0;

  for (;;)
  { // Repeat forever (or until a 'break' or 'return')
    for (int i = 0; i < strip.numPixels(); i++)
    {                                     // For each pixel in strip...
      if (((i >= tail) && (i <= head)) || //  If between head & tail...
          ((tail > head) && ((i >= tail) || (i <= head))))
      {
        strip.setPixelColor(i, strip.Color(0, 0, 0, 255)); // Set white
      }
      else
      { // else set rainbow
        int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
        strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
      }
    }

    ownshow(); // Update strip with new contents
    // There's no delay here, it just runs full-tilt until the timer and
    // counter combination below runs out.

    firstPixelHue += 40; // Advance just a little along the color wheel

    if ((millis() - lastTime) > whiteSpeed)
    { // Time to update head/tail?
      if (++head >= strip.numPixels())
      { // Advance head, wrap around
        head = 0;
        if (++loopNum >= loops)
          return;
      }
      if (++tail >= strip.numPixels())
      { // Advance tail, wrap around
        tail = 0;
      }
      lastTime = millis(); // Save time of last movement
    }
  }
}

void pulseWhite(uint8_t wait, uint8_t start, uint8_t end)
{
  const uint8_t c = 32;
  for (int j = start; j < end; j++)
  { // Ramp up from 0 to 255
    // Fill entire strip with white at gamma-corrected brightness level 'j':
    strip.fill(strip.Color(c, c, c, strip.gamma8(j)));
    ownshow();
    delay(wait);
  }

  for (int j = end; j >= start; j--)
  { // Ramp down from 255 to 0
    strip.fill(strip.Color(c, c, c, strip.gamma8(j)));
    ownshow();
    delay(wait);
  }
}

void rainbowFade2White(int wait, int rainbowLoops, int whiteLoops)
{
  int fadeVal = 0, fadeMax = 100;

  // Hue of first pixel runs 'rainbowLoops' complete loops through the color
  // wheel. Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to rainbowLoops*65536, using steps of 256 so we
  // advance around the wheel at a decent clip.
  for (uint32_t firstPixelHue = 0; firstPixelHue < rainbowLoops * 65536;
       firstPixelHue += 256)
  {

    for (int i = 0; i < strip.numPixels(); i++)
    { // For each pixel in strip...

      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      uint32_t pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());

      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the three-argument variant, though the
      // second value (saturation) is a constant 255.
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue, 255,
                                                          255 * fadeVal / fadeMax)));
    }

    ownshow();
    delay(wait);

    if (firstPixelHue < 65536)
    { // First loop,
      if (fadeVal < fadeMax)
        fadeVal++; // fade in
    }
    else if (firstPixelHue >= ((rainbowLoops - 1) * 65536))
    { // Last loop,
      if (fadeVal > 0)
        fadeVal--; // fade out
    }
    else
    {
      fadeVal = fadeMax; // Interim loop, make sure fade is at max
    }
  }

  for (int k = 0; k < whiteLoops; k++)
  {
    for (int j = 0; j < 256; j++)
    { // Ramp up 0 to 255
      // Fill entire strip with white at gamma-corrected brightness level 'j':
      strip.fill(strip.Color(255, 255, 255, strip.gamma8(j)));
      ownshow();
    }
    delay(1000); // Pause 1 second
    for (int j = 255; j >= 0; j--)
    { // Ramp down 255 to 0
      strip.fill(strip.Color(255, 255, 255, strip.gamma8(j)));
      ownshow();
    }
  }

  delay(500); // Pause 1/2 second
}
