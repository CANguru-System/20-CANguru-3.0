#include "Core_Logging.h"
#include <Arduino.h>
#include "protocol_constants_core.h"

void coreLoggingInit() {
    Serial.begin(MONITOR_SPEED);
    delay(500);
    Serial.println();
    Serial.println("C A N g u r u - D e c o d e r");
    Serial.printf("on %s\r\n", ARDUINO_BOARD);
    Serial.printf("CPU Frequency = %d MHz\r\n", F_CPU / 1000);
}

void coreLog(const char* msg) {
    Serial.println(msg);
}
