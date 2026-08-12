#include "Core_globals.h"
#include "Core_Config.h"
#include "Core_alive.h"
#include "Core_EspNow.h"
#include <string.h>
#include "protocol_constants_core.h"

const char *key_id = "id";
const char *key_otaPending = "otaPending";
const char *key_bridgeMac = "bridge_mac";

void coreConfigInit()
{
    prefs.init();
    uint8_t Dec_assignedId = prefs.getByte(key_id, INVALID_ASSIGNED_ID);
    aliveSetDecoderID(Dec_assignedId);

    Serial.printf("Loaded assigned ID from preferences: %d\r\n", aliveGetDecoderID());

    if (coreConfigIsOtaPending())
    {
        Serial.println("OTA pending, keeping existing assigned ID and settings");
        coreConfigClearOtaPending();
        prefs.getBytes(key_bridgeMac, Bridge_Mac, 6);
        bridge_is_known = true;
        logMac("Loaded bridge MAC from preferences: ", Bridge_Mac);
    }
}

bool coreConfigIsOtaPending()
{
    return prefs.getBool(key_otaPending, false);
}

void coreConfigSetOtaPending(bool v)
{
    prefs.putBool(key_otaPending, v);
}

void coreConfigClearOtaPending()
{
    coreConfigSetOtaPending(false);
}
