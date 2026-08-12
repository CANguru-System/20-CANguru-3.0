#pragma once

#include <Arduino.h>
#include "protocol_constants_core.h"
#include "Bridge_Decoder.h"

extern bool GB_found;
struct ToGUICANFrameItem
{
  uint32_t seq;
  uint8_t data[CAN_FRAME_SIZE];
  char source;
};

struct ToCANItem
{
  uint8_t data[CAN_FRAME_SIZE];
  bool hash;
};

const uint8_t uid_num = 4;
extern uint8_t uid_device[4];


// ----------------------------------------------------
// CAN-Funktionen
// ----------------------------------------------------

void initTWAI();

// ----------------- Senderichtung von Bridge oder WDP zur Gleisbox

// sendet einen CAN-Frame an das CAN-Interface
// void sendCanFrame(uint8_t *data, bool hash);

// entnimmt CAN-Frames aus dem FiFo und ruft sendCanFrame auf
// wird nur von der loop() aufgerufen
void processCANFrames2CAN();

// nimmt einen CAN-Frame aus der FiFo
// bool dequeueCANFrame2CAN(ToCANItem &item);

// schreibt einen CAN-Frame in die ausgehende FIFO
void enqueueCANFrame2CAN(const uint8_t *data, bool hash);


// L----------------- Leserichtung von der Gleisbox an WDP oder die GUI

// Liest einen CAN-Frame von der Gleisbox ein
void readCANMsg();

//  CAN-Frames zum CAN-Interface zu schicken und
//  CAN-Frames zum GUI anzeigen zu lassen.
void enqueueCanToFifo(const uint8_t *buf, char c);

// nimmt einen CAN-Frame aus der FiFo
bool dequeueCanFromFifo(ToGUICANFrameItem &item);

// an die GUI senden
void sendFiFoFromCANToGUI();

