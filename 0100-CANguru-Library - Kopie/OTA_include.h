
#ifndef OTA_INCLUDE
#define OTA_INCLUDE

#ifdef ESP32_DECODER

const uint8_t numChars = 32;
// EEPROM-Adressen
#define setup_done 0x47
#define setup_NOT_done 0x00

const uint8_t startWithOTA = 0x77;
const uint8_t startWithoutOTA = 0x55;

void Connect2WiFiandOTA(Preferences preferences);
#endif
#endif
