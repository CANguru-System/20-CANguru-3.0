#pragma once

#include <vector>
#include "CANguruPreferences.h"

// Globale Deklaration (NICHT instanziert!)
extern CANguruPreferences prefs;
#if DECODER_TYPE == DEVTYPE_POWER
extern CANguruPreferences prefsPower;
#endif
#if DECODER_TYPE == DEVTYPE_SIGNAL
extern CANguruPreferences prefsSignal;
#endif
#if DECODER_TYPE == DEVTYPE_SWITCH
extern CANguruPreferences prefsSwitch;
#endif
#if DECODER_TYPE == DEVTYPE_RM
extern CANguruPreferences prefsFeedBack;
#endif
// Funktion stellt sicher, dass keine unerlaubten 8-Bit-Werte geladen werden können
uint8_t readValfromPreferences8(CANguruPreferences& pref, const char* key, uint8_t val, uint8_t min, uint8_t max);

// Funktion stellt sicher, dass keine unerlaubten 16-Bit-Werte geladen werden können
uint16_t readValfromPreferences16(CANguruPreferences& pref, const char* key, uint16_t val, uint16_t min, uint16_t max);

// Mit testMinMax wird festgestellt, ob ein Wert innerhalb der
// Grenzen von min und max liegt
bool testMinMax(uint16_t oldval, uint16_t val, uint16_t min, uint16_t max);

bool parsePacket(const uint8_t* data, int len,
                 uint8_t& featureId,
                 uint8_t& commandId,
                 const uint8_t*& payload,
                 uint8_t& payloadLen);
