#include <Arduino.h>
#include <vector>

# pragma once

// OTA‑Zustände
enum OtaState {
    OTA_IDLE = 0,           // nichts aktiv
    OTA_WAIT_FOR_FIRMWARE,  // Upload läuft noch
    OTA_START,              // OTA für aktuellen Decoder starten
    OTA_WAIT_FOR_FINISH,    // auf Erfolg/Timeout warten
    OTA_NEXT,               // nächster Decoder in Queue
    OTA_DONE                // alles fertig
};

extern OtaState otaState;
extern uint16_t expectedSeq;
extern File fwFile;
extern bool firmwareReady;
extern size_t firmwareSizeOnFs;

// Datenstruktur: OtaEntry + nkaa::List
struct OtaEntry {
    uint8_t decoderId;
    uint8_t mac[6];
};

extern std::vector<OtaEntry> otaQueue;

extern OtaState otaState;
extern OtaEntry currentOta;

extern uint8_t otaTotalCount;
extern uint8_t otaFinishedCount;
extern uint8_t currentPercent;
extern unsigned long otaTimer;

extern bool otaSuccessReceived;

void startGroupOtaForType(uint8_t targetType);
void processOta();
void processFirmwareQueue();
void processAckQueue();
void onUploadFinished(size_t size);
