#pragma once

#include <Arduino.h>
#include "globals.h"

// Datenstruktur für ausstehende Assigns
struct PendingAssign {
    uint8_t mac[6];
    uint8_t assignedId;
    uint8_t decoderType;
    uint8_t retriesLeft;
    uint32_t nextRetryMillis;
    bool waitingForAck;
};
void processAssignRetries();
void cleanupFinishedAssigns();

void InitAlive();
// ----------------------------------------------------
// Alive-Überwachung
// ----------------------------------------------------
void checkAliveStates();
// ----------------------------------------------------
// Alive-Paket senden
// Format-Beispiel: BRIDGE_ALIVE, counter, resetFlag, reserved
// ----------------------------------------------------

void sendDecoderAddedToGUI(const DecoderInfo &info);
void handleHelloPacket(const uint8_t* mac,
                       const uint8_t* payload,
                       uint8_t len);
void handleReadyPacket(const uint8_t* mac,
                       const uint8_t* payload,
                       uint8_t len);
void handleAlivePacket(const uint8_t* mac,
                       const uint8_t* payload,
                       uint8_t len);
void handlePowerPacket(const uint8_t* mac,
                       uint8_t commandId,
                       const uint8_t* payload,
                       uint8_t payloadLen);
void handleSwitchSignalPacket(const uint8_t* mac,
                        uint8_t featureId,
                       uint8_t commandId,
                       const uint8_t* payload,
                       uint8_t payloadLen);
void handleSettingPacket(const uint8_t* mac,
                       uint8_t commandId,
                       const uint8_t* payload,
                       uint8_t payloadLen);
                       
void checkAliveTimeouts();
void sendAliveSummary();

const unsigned long ALIVE_SUMMARY_INTERVAL_MS = 1000;
const unsigned long ALIVE_INTERVAL = 1000; // 1 Sekunde
extern unsigned long lastAliveSent;