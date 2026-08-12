#pragma once

#include <Arduino.h>
#include <WiFiUdp.h>
#include <ESPAsyncWebServer.h>
#include <vector>

extern WiFiUDP udp;
extern IPAddress guiIP;
extern IPAddress wdpIP;
extern bool resetFlag; // nach Reset: GUI muss sich neu synchronisieren
extern unsigned long lastRequest;
extern unsigned long lastAliveSummary;
extern bool guiIsAlive;
extern unsigned long lastGuiSeen;
extern unsigned long lastGuiAlive;
extern unsigned long lastPingSend;
extern bool GleisboxFound;
const bool hashEnabled = true;
const bool hashDisabled = false;

extern bool systemReady;

extern WiFiUDP udpWDP;
extern WiFiUDP udpGUI; // GUI -> Bridge

extern AsyncWebServer otaServer;
extern const char *FW_PATH;

const uint8_t Bridge_Mac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

extern uint8_t SysSTOPP[];

struct DecoderInfo
{
  uint8_t assignedId;
  uint8_t Decoder_Mac[6];
  uint8_t decoderType;
  uint8_t fwMajor;
  uint8_t fwMinor;
  uint8_t value0;
  uint8_t value1;
  bool isReady;
  unsigned long lastSeen;
};

extern std::vector<DecoderInfo> decoders;

extern uint8_t FW_MAJOR;
extern uint8_t FW_MINOR;

bool GUIipIsNotSet();
void logMac(const char *prefix, const uint8_t *mac);
int findDecoderByMac(const uint8_t *mac);
void loadDecoderList();
uint8_t getDecoderIdForMac(const uint8_t mac[6]);
int addDecoder(const uint8_t *mac, const uint8_t* pkt);

