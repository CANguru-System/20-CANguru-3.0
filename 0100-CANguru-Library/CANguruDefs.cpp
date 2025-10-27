
#include <Arduino.h>
#include "Preferences.h"
#include "CANguruDefs.h"

//  Liest die IP-Adresse ein, die vorher auf den Chip geschrieben wurde
IPAddress readIP(Preferences& pref)
{
  if (pref.isKey("ssid"))
    log_d("SSID OK");
  if (pref.isKey("password"))
    log_d("PASSWORD OK");
  if (pref.isKey("IP0"))
    log_d("IP-ADDRESS OK");
  uint32_t ip = pref.getUInt("IP0", 0); // Default = 0.0.0.0
  IPAddress IPAdr(ip);
  log_d("Gespeicherte IP-Adresse: %d.%d.%d.%d", IPAdr[0], IPAdr[1], IPAdr[2], IPAdr[3]);
  return IPAdr;
}

// Funktion stellt sicher, dass keine unerlaubten 8-Bit-Werte geladen werden können
uint8_t readValfromPreferences(Preferences& preferences, const char* key, uint8_t val, uint8_t min, uint8_t max)
{
  uint8_t v = preferences.getUChar(key, val);
  if ((v >= min) && (v <= max))
    return v;
  else
  {
    preferences.putUChar(key, val);
    return val;
  }
}

// Funktion stellt sicher, dass keine unerlaubten 16-Bit-Werte geladen werden können
uint16_t readValfromPreferences16(Preferences& preferences, const char* key, uint16_t val, uint16_t min, uint16_t max)
{
  uint16_t v = preferences.getUShort(key, val);
  if ((v >= min) && (v <= max))
    return v;
  else
  {
    preferences.putUShort(key, val);
    return val;
  }
}

// Mit testMinMax wird festgestellt, ob ein Wert innerhalb der
// Grenzen von min und max liegt
bool testMinMax(uint16_t oldval, uint16_t val, uint16_t min, uint16_t max)
{
  return (oldval != val) && (val >= min) && (val <= max);
}

char highbyte2char(int num){
  num /= 10;
  return char ('0' + num);
};

// converts lowbyte of integer to char
char lowbyte2char(int num){
  num = num - num / 10 * 10;
  return char ('0' + num);
};

uint8_t oneChar(uint16_t val, uint8_t pos) {
	char buffer [5];
	itoa (val, buffer, 10);
	return buffer[4 - pos];
}

uint8_t dev_type = DEVTYPE_BASE;

uint8_t hex2dec(uint8_t h){
  return h / 16 * 10 + h % 16;
}

