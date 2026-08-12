#pragma once

#include <Arduino.h>

// -------------------- Senden von WDP -------------------
void initWdpQueue();
void wdpSendTask(void *pv);
void sendToWDP(const uint8_t *buffer);

// -------------------- Empfangen von WDP -------------------
void readWDP();

// -------------------- lokomotive.cs2 übertragen -------------------
void setupDownloadRoutesCS2();
void setupUploadRoute();
void onRequest(AsyncWebServerRequest *request);
void iNetEvtCB(arduino_event_id_t event, arduino_event_info_t info);