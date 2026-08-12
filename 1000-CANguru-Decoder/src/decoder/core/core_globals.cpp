#include <Arduino.h>
#include <vector>
#include <esp_now.h>
#include "Core_globals.h"
#include "Core_Config.h"
#include "Core_EspNow.h"
#include "CANguruPreferences.h"

// Keys für NVS

// Hier wird die Instanz EINMAL erzeugt
CANguruPreferences prefs("decoder");

#if DECODER_TYPE == DEVTYPE_POWER
CANguruPreferences prefsPower("power");
#endif
#if DECODER_TYPE == DEVTYPE_SIGNAL
CANguruPreferences prefsSignal("signal");
#endif
#if DECODER_TYPE == DEVTYPE_SWITCH
CANguruPreferences prefsSwitch("switch");
#endif
#if DECODER_TYPE == DEVTYPE_RM
CANguruPreferences prefsFeedBack("feedback");
#endif


bool parsePacket(const uint8_t* data, int len,
                 uint8_t& featureId,
                 uint8_t& commandId,
                 const uint8_t*& payload,
                 uint8_t& payloadLen)
{
  // Feature | Frame | Payload
    if (len < 2)
        return false;

    featureId = data[0];
    commandId = data[1];

    payloadLen = len - 2;

    if (payloadLen > 0)
        payload = &data[2];
    else
        payload = nullptr;

    return true;
}

uint8_t readValfromPreferences8(CANguruPreferences& pref, const char* key, uint8_t val, uint8_t min, uint8_t max)
{
  uint8_t v = pref.getByte(key, val); // 1000 ist der Default-Wert, falls der Schlüssel nicht existiert
  if ((v >= min) && (v <= max))
    return v;
  else
  {
    pref.putByte(key, val);
    return val;
  }
}

uint16_t readValfromPreferences16(CANguruPreferences& pref, const char* key, uint16_t val, uint16_t min, uint16_t max)
{
  uint16_t v = pref.getUInt(key, val); // 1000 ist der Default-Wert, falls der Schlüssel nicht existiert
  if ((v >= min) && (v <= max))
    return v;
  else
  {
    pref.putUInt(key, val);
    return val;
  }
}

// Mit testMinMax wird festgestellt, ob ein Wert innerhalb der
// Grenzen von min und max liegt
bool testMinMax(uint16_t oldval, uint16_t val, uint16_t min, uint16_t max)
{
  return (oldval != val) && (val >= min) && (val <= max);
}
