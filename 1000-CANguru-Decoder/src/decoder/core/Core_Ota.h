#pragma once
#include <stdint.h>

extern volatile bool otaRequested;
extern bool otaInProgress;
extern bool otaRunning;

void coreOtaInit();
void coreOtaLoop();
bool coreOtaStartFromBridge();
