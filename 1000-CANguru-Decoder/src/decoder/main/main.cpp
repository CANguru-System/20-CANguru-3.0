#include <Arduino.h>
#include "soc/rtc_cntl_reg.h"
#include "../core/DecoderBase.h"
#include "DecoderFactory.h"
#include "../core/Core_EspNow.h"
#include "../core/Core_Config.h"
#include "protocol_constants_core.h"
#include "../core/Core_Alive.h"

//extern DecoderBase* gDecoder;

// Decoder/Slaves registieren nur die Adresse des Masters, von dem sie den Broadcast empfangen haben. Alle Antworten gehen an diese Adresse zurück.
// Es können also nur Slaves mit einem Master kommunizieren, nicht aber untereinander.
// Die Broadcast-Adresse wird beim ersten Empfang registriert.
/*void setup() {
    pinMode(GPIO_NUM_8, OUTPUT);
}

void loop() {
    digitalWrite(GPIO_NUM_8, HIGH);
    delay(500);
    digitalWrite(GPIO_NUM_8, LOW);
    delay(500);
}
*/
void setup() {
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable brownout detector
/*    Serial.begin(bdrMonitor);
    delay(350);
    Serial.println("DECODER: setup()");

    coreEspNowInit();*/          // ESP-NOW + Bridge-Peer + MAC
    aliveInit();
    gDecoder = createDecoder();
    if (gDecoder) {
        gDecoder->setup();
    }
}

void loop() {
    if (gDecoder)
        gDecoder->loop();
     aliveLoop();            // Alive separat
}
