
#include <Arduino.h>

uint8_t LED_BUILTIN_OWN;

void LED_begin(uint8_t LED_)
{
#if defined(ESP32_DECODER) || defined(ESP32_SCANNER) || ESP32C3_LIGHT
  LED_BUILTIN_OWN = LED_;
  pinMode(LED_BUILTIN_OWN, OUTPUT);
  // turn the LED off by making the voltage LOW
  digitalWrite(LED_BUILTIN_OWN, LOW);
#endif
}

void LED_on()
{
#if defined(ESP32_DECODER) || defined(ESP32_SCANNER) || ESP32C3_LIGHT
  digitalWrite(LED_BUILTIN_OWN, HIGH);
#endif
}

void LED_off()
{
#if defined(ESP32_DECODER) || defined(ESP32_SCANNER) || ESP32C3_LIGHT
  digitalWrite(LED_BUILTIN_OWN, LOW);
#endif
}
