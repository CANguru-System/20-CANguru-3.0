#include "Core_Feature.h"
#include "generated_constants/protocol_constants_core.h"

// Hier zentral definieren, welche Typen als Feature-Frames gelten.
// Aktuell: PowerDecoder-Bereich.
bool isFeatureFrame(uint8_t type)
{
    switch (type)
    {
    case POWER_CMD_SET_THRESHOLD:
    case POWER_CMD_GET_THRESHOLD:
    case POWER_CMD_GET_VERSION:
    case POWER_CMD_SHUTDOWN:
    case SWITCH_CMD_SET_SETTINGS:
    case SWITCH_CMD_GET_SETTINGS:
    case SIGNAL_CMD_SET_SETTINGS:
    case SIGNAL_CMD_GET_SETTINGS:
        return true;

    default:
        return false;
    }
}
