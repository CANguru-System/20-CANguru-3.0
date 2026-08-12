#include <Arduino.h>
#include <WiFiUdp.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <vector>
#include <esp_now.h>
#include "protocol_constants_core.h"
#include "globals.h"
#include "alive.h"

AsyncWebServer otaServer(80);
const char *FW_PATH = "/firmware.bin";

IPAddress guiIP(0, 0, 0, 0);
// IP von WinDigiPet-PC
IPAddress wdpIP(0, 0, 0, 0);

bool resetFlag = true; // nach Reset: GUI muss sich neu synchronisieren
unsigned long lastRequest = 0;
unsigned long lastAliveSummary = 0;
bool guiIsAlive = false;
unsigned long lastGuiSeen = 0;
unsigned long lastGuiAlive = 0;
unsigned long lastPingSend = 0;
bool systemReady = false;

uint8_t SysSTOPP[] =    {0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// -------------------- UDP-Objekte ----------------------
WiFiUDP udp;
WiFiUDP udpWDP;
WiFiUDP udpGUI; // falls du ihn wirklich nutzt

// Set web server port number to 80
bool GleisboxFound;

std::vector<DecoderInfo> decoders;

uint8_t FW_MAJOR = 4;
uint8_t FW_MINOR = 0;

// GUI-Kommunikation
bool GUIipIsNotSet()
{
    return guiIP == IPAddress(0, 0, 0, 0);
}

void logMac(const char *prefix, const uint8_t *mac)
{
    Serial.print(prefix);
    for (int i = 0; i < 6; i++)
    {
        Serial.printf("%02X", mac[i]);
        if (i < 5)
            Serial.print(":");
    }
    Serial.println();
}

int findDecoderByMac(const uint8_t *mac)
{
    for (size_t i = 0; i < decoders.size(); i++)
    {
        if (memcmp(decoders[i].Decoder_Mac, mac, 6) == 0)
            return i;
    }
    return -1;
}

uint8_t g_nextId = 1;
static const char *DECODER_LIST_FILE = "/decoders.txt";

void loadDecoderList()
{
    decoders.clear();
    g_nextId = 1;

    if (!LittleFS.exists(DECODER_LIST_FILE))
    {
        Serial.printf("Decoder list not found, nextId=%d\r\n", g_nextId);
        return;
    }

    File f = LittleFS.open(DECODER_LIST_FILE, "r");
    if (!f)
    {
        Serial.println("Fehler: Decoder-Liste konnte nicht geoeffnet werden!");
        return;
    }

    while (f.available())
    {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.length() == 0)
            continue;

        DecoderInfo e;
        int id;
        int type;
        int m0, m1, m2, m3, m4, m5;

        int n = sscanf(line.c_str(),
                       "%x:%x:%x:%x:%x:%x,%d,%x",
                       &m0, &m1, &m2, &m3, &m4, &m5, &id, &type);
if (n == 8)
{
    e.Decoder_Mac[0] = (uint8_t)m0;
    e.Decoder_Mac[1] = (uint8_t)m1;
    e.Decoder_Mac[2] = (uint8_t)m2;
    e.Decoder_Mac[3] = (uint8_t)m3;
    e.Decoder_Mac[4] = (uint8_t)m4;
    e.Decoder_Mac[5] = (uint8_t)m5;
    e.assignedId = (uint8_t)id;
    e.decoderType = (uint8_t)type;

    decoders.push_back(e);

    if (e.assignedId >= g_nextId)
        g_nextId = e.assignedId + 1;
}
    }

    f.close();

    Serial.printf("Decoder list loaded, %d entries, nextId=%d\r\n",
                  decoders.size(), g_nextId);
}

void saveDecoderList()
{
    File f = LittleFS.open(DECODER_LIST_FILE, "w");
    if (!f)
    {
        Serial.println("Fehler: Decoder-Liste konnte nicht geoeffnet werden!");
        return;
    }

    for (size_t i = 0; i < decoders.size(); ++i)
    {
        const DecoderInfo &d = decoders[i];
        f.printf("%02X:%02X:%02X:%02X:%02X:%02X,%d,%02X\n",
                 d.Decoder_Mac[0], d.Decoder_Mac[1], d.Decoder_Mac[2],
                 d.Decoder_Mac[3], d.Decoder_Mac[4], d.Decoder_Mac[5],
                 d.assignedId, d.decoderType);
    }

    f.close();
}

int addDecoder(const uint8_t *mac, const uint8_t *pkt)
{
    uint8_t assignedId = pkt[0];
    uint8_t decoderType = pkt[1];

    DecoderInfo info{};

    if (assignedId == INVALID_ASSIGNED_ID)
    {
        info.assignedId = g_nextId;
        g_nextId++;
    }
    else
    {
        // Decoder meldet seine bestehende ID
        info.assignedId = assignedId;
    }

    info.decoderType = decoderType;
    memcpy(info.Decoder_Mac, mac, 6);
    info.isReady = false;
    info.lastSeen = millis();
    
    decoders.push_back(info);
//    saveDecoderList();

    return decoders.size() - 1;
}

uint8_t getDecoderIdForMac(const uint8_t mac[6])
{

    // 1. existiert schon?
    for (size_t i = 0; i < decoders.size(); ++i)
    {
        const DecoderInfo &d = decoders[i];
        if (memcmp(d.Decoder_Mac, mac, 6) == 0)
            return d.assignedId;
    }

    // 2. neu anlegen
    DecoderInfo e;
    memcpy(e.Decoder_Mac, mac, 6);
    e.assignedId = g_nextId++;

    decoders.push_back(e);
    saveDecoderList();

    return e.assignedId;
}
