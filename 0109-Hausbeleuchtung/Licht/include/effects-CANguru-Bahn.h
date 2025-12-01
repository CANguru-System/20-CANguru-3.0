
#ifndef EFFECTS_H
#define EFFECTS_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

extern const uint8_t LED_COUNT_NORM;
void ownshow();
void colorWipe(uint32_t c, uint8_t wait);
void theaterChase(uint32_t c, uint8_t wait);
void rainbow(uint8_t wait);
void rainbowCycle(uint8_t wait);
void theaterChaseRainbow(uint8_t wait);
void whiteOverRainbow(int whiteSpeed, int whiteLength);
void pulseWhite(uint8_t wait, uint8_t start, uint8_t end);
void rainbowFade2White(int wait, int rainbowLoops, int whiteLoops);
void own_clear();

#endif