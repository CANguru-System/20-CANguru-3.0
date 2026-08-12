#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <nvs.h>
#include <nvs_flash.h>
#include "Core_EspNow.h"
#include "esp_wifi.h"
#include "protocol_constants_core.h"
#include "DecoderBase.h"
#include "Core_Config.h"
#include "Core_Logging.h"
#include "DecoderBase.h"
#include "Core_Feature.h"
#include "Core_Alive.h" // für decoderID
#include "Core_globals.h"
#include "DecoderResetUtility.h"
#if DECODER_TYPE == DEVTYPE_SIGNAL
#include "SignalDecoder.h"
#endif
#if DECODER_TYPE == DEVTYPE_SWITCH
#include "SwitchDecoder.h"
#endif
#if DECODER_TYPE == DEVTYPE_POWER
#include "PowerDecoder.h"
#endif
#if DECODER_TYPE == DEVTYPE_RM
#include "FeedbackDecoder.h"
#endif
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

QueueHandle_t espnowQueue;

DecoderBase *gDecoder = nullptr;
BeforeRestartCallback beforeRestartCallback = nullptr;
AfterstartCallback afterstartCallback = nullptr;

uint8_t Bridge_Mac[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // wird beim ersten Kontakt mit der tatsächlichen MAC-Adresse gefüllt
uint8_t Decoder_Mac[6];
bool espNowReady = false;
bool bridge_is_known = false;
uint8_t nextType = MSG_START_DECODERS;

uint8_t uid_device[uid_num];
uint8_t hasharr[2];

uint8_t DEVTYPE = DEVTYPE_BASE;
uint8_t VERS_HIGH = 0;
uint8_t VERS_LOW = 0;

extern volatile bool otaRequested;
extern bool otaInProgress;
extern bool otaRunning;
extern uint8_t nextType;

static bool startAllowed = false;

static ConfigProvider_t configProvider;

void registerConfigProvider(const ConfigProvider_t *provider)
{
    configProvider = *provider;
}

static bool wrong_nextType(uint8_t type)
{
    return type != nextType;
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

void coreEspNowSendCAN2Bridge(const uint8_t *data)
{
    if (!espNowReady || !bridge_is_known)
        return;

    xQueueSend(espnowQueue, data, portMAX_DELAY);
}

void generateHash()
{
#define UID_BASE 0x45009195ULL // CAN-UID
    //  uint32_t uid = UID_BASE + (DEVTYPE - DEVTYPE_BASE - 1) * maxdevice + offset;
    uint32_t uid = UID_BASE + aliveGetDecoderID(); // UID_BASE + offset;
    uid_device[0] = (uint8_t)(uid >> 24);
    uid_device[1] = (uint8_t)(uid >> 16);
    uid_device[2] = (uint8_t)(uid >> 8);
    uid_device[3] = (uint8_t)uid;
    uint16_t highbyte = uid >> 16;
    uint16_t lowbyte = uid;
    uint16_t hash = highbyte ^ lowbyte;
    bitWrite(hash, 7, 0);
    bitWrite(hash, 8, 1);
    bitWrite(hash, 9, 1);
    hasharr[0] = hash >> 8;
    hasharr[1] = hash;
}

// sendPING ist die Antwort der Decoder auf eine PING-Anfrage
void sendPING()
{
    uint8_t opFrame[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    // D-00(31)D716 R [8] 45 00 91 96 01 00 00 54    E......TRE: 31 Beispiel POWER-Decoder
    opFrame[CANcmd] = PING_R;
    opFrame[hash0] = hasharr[0];
    opFrame[hash1] = hasharr[1];
    opFrame[Framelng] = 0x08;

    for (uint8_t i = 0; i < uid_num; i++)
    {
        opFrame[i + 5] = uid_device[i];
    }
    opFrame[data4] = VERS_HIGH;
    opFrame[data5] = VERS_LOW;
    opFrame[data6] = DEVTYPE >> 8;
    opFrame[data7] = DEVTYPE;
    //%
    coreEspNowSendCAN2Bridge(opFrame);
}

void copyStringToFrame(uint8_t *frame, int start, const String &txt)
{
    memcpy(&frame[start], txt.c_str(), txt.length());
}

void sendConfig()
{
    /*
    Format Gerätebeschreibung
      Format Gerätebeschreibung:
      Typ     Bedeutung
      Char    Anzahl der Messwerte im Gerät.
      Char    Anzahl der Konfigurationskanäle
      2 Byte  frei.
      U32     Seriennummer CS2.
      String  8 Byte Artikelnummer.
      String  Gerätebezeichnung, \0 Terminiert
  */

    const uint8_t ROWS = 5;
    uint8_t opFrame[] = {0x00, 0x3B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    const char *line0 = configProvider.getLine0();
    const char *line1 = configProvider.getLine1();
    for (uint8_t line = 0; line < ROWS; line++) // 0 bis 4
    {
        switch (line)
        {
        case 0:
            opFrame[hash0] = 0x03;
            opFrame[hash1] = line + 1;
            opFrame[Framelng] = 0x08;
            // Index
            // Unter Index 0 sind die Gerätebeschreibung abrufbar. Primär ist dies die Anzahl der zur Verfügung
            // gestellten Messkanäle. Weiterhin enthalten sind Angaben zur Identifikation des Gerätes.
            // Anzahl Kanäle
            opFrame[data1] = 0x01;
            opFrame[data7] = aliveGetDecoderID();
            break;
        case 1:
            for (uint8_t n = 0; n < 4; n++)
            {
                opFrame[5 + n * 2] = uid_device[n] / 16 + 0x30;
                opFrame[6 + n * 2] = uid_device[n] % 16 + 0x30;
            }
            break;
        case 2:
            copyStringToFrame(opFrame, 5, line0);
            break;
        case 3:
            copyStringToFrame(opFrame, 5, line1);
            opFrame[0x0C] = 0x00;
            break;
        case 4:

            opFrame[hash0] = hasharr[0];
            opFrame[hash1] = hasharr[1];
            opFrame[Framelng] = 0x06;
            opFrame[data0] = uid_device[0];
            opFrame[data1] = uid_device[1];
            opFrame[data2] = uid_device[2];
            opFrame[data3] = uid_device[3];
            opFrame[data4] = 0x00;
            opFrame[data5] = ROWS - 1;
            opFrame[data6] = 0x00;
            opFrame[data7] = 0x00;
            break;
        }
        coreEspNowSendCAN2Bridge(opFrame);
        //%
        //    gDecoder->sendToBridge(FEATURE_CS2, FRAME_CAN_2BRIDGE, opFrame, sizeof(opFrame));
    }
}

static void onDecDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    (void)mac_addr;
    (void)status;
}

// einghende Daten von der Bridge über ESP-NOW
static void onDecDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
    if (!gDecoder)
        return;
    /*
Protokoll der empfangenen Daten:
[0] SendMode
[1] Selector
[2] FeatureID
[3] CommandID
[4..] Payload
*/
    if (bridge_is_known == false)
    {
        logMac("Received first ESP-NOW packet from unknown MAC: ", mac);
        // Broadcast-Peer eintragen
        esp_now_peer_info_t bcast{};
        memcpy(&Bridge_Mac[0], mac, 6);
        memcpy(bcast.peer_addr, Bridge_Mac, 6);
        bcast.channel = BRIDGE_AP_CHANNEL;
        bcast.ifidx = WIFI_IF_STA;
        bcast.encrypt = false;
        esp_now_add_peer(&bcast);
        // für rejoin adresse speichern
        prefs.putBytes(key_bridgeMac, Bridge_Mac, 6);
        bridge_is_known = true;
    }

    // Paket an Decoder-Logik weiterreichen
    gDecoder->onEspNowDataReceived(incomingData, len);
}

void initEspNowQueue()
{
    espnowQueue = xQueueCreate(30, CAN_FRAME_SIZE);
    if (espnowQueue == NULL)
    {
        Serial.println("ERROR: Could not create ESP-NOW queue!");
    }
}

void espnowSendTask(void *pv)
{
    uint8_t frame[CAN_FRAME_SIZE];

    for (;;)
    {
        // Blockiert, bis ein Frame in der Queue liegt
        if (xQueueReceive(espnowQueue, frame, portMAX_DELAY))
        {

            gDecoder->sendToBridge(FEATURE_CS2, FRAME_CAN_2BRIDGE, frame, CAN_FRAME_SIZE);

            /*    // Retry-Mechanismus
                for (int retry = 0; retry < 5; retry++)
                { //%
                    if (sendPacket(pkt))
                        break;

                    // kurze Pause, damit der WiFi-Stack Luft bekommt
                    vTaskDelay(5 / portTICK_PERIOD_MS);
                }*/

            // kleine Pause, damit ESP-NOW nicht überlastet
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
    }
}

void coreEspNowInit()
{
    // 1. WiFi starten
    WiFi.disconnect(true, true);
    delay(100);

    WiFi.mode(WIFI_STA);
    delay(150);

    // 2. Kanal setzen
    esp_wifi_set_ps(WIFI_PS_NONE);
    esp_wifi_set_channel(BRIDGE_AP_CHANNEL, WIFI_SECOND_CHAN_NONE);
    delay(50);

    // 3. MAC-Adresse auslesen
    esp_read_mac(Decoder_Mac, ESP_MAC_WIFI_STA);
    Serial.printf("DECODER-coreEspNowInit: WiFi STA MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n",
                  Decoder_Mac[0], Decoder_Mac[1], Decoder_Mac[2],
                  Decoder_Mac[3], Decoder_Mac[4], Decoder_Mac[5]);

    // 4. Jetzt erst ESP-NOW initialisieren
    esp_now_deinit();
    delay(20);

    if (esp_now_init() != ESP_OK)
    {
        Serial.println("DECODER-coreEspNowInit: ESP-NOW init failed");
        return;
    }

    espNowReady = true;

    // 6. Callbacks registrieren
    esp_now_register_recv_cb(onDecDataRecv);
    esp_now_register_send_cb(onDecDataSent);

 // 🔴 HIER: Peer für Bridge anlegen, falls bekannt
    if (bridge_is_known)
    {
        esp_now_peer_info_t peer{};
        memcpy(peer.peer_addr, Bridge_Mac, 6);
        peer.channel = BRIDGE_AP_CHANNEL;
        peer.ifidx   = WIFI_IF_STA;
        peer.encrypt = false;

        esp_err_t pr = esp_now_add_peer(&peer);
        Serial.printf("DECODER-coreEspNowInit: add_peer result=%d\r\n", pr);
    }
    
    initEspNowQueue();
    xTaskCreate(espnowSendTask, "espnowSendTask", 4096, NULL, 1, NULL);
}

void sendHello(uint8_t assignedId, uint8_t decoderType, const uint8_t *mac)
{
    const uint8_t len = 8;
    uint8_t payload[len] = {
        assignedId,
        decoderType,
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]};
    gDecoder->sendToBridge(FEATURE_ALIVE, FRAME_DECODER_HELLO, payload, len);
}

void startHelloSequence()
{
    uint8_t id = aliveGetDecoderID();
    if (!startAllowed || otaInProgress)
        return;
    startAllowed = false;
    Serial.printf("Starting HELLO sequence with ID=%d\r\n", id);
    sendHello(id, DEVTYPE, Decoder_Mac);
}

void sendRejoin()
{
    const uint8_t len = 1;
    uint8_t pkt[len];
    pkt[0] = aliveGetDecoderID();

Serial.printf("Sending REJOIN with ID=%d\r\n", pkt[0]);
    gDecoder->sendToBridge(FEATURE_DECODER, FRAME_DECODER_REJOIN, pkt, len);
}

void resetDecoderNVS()
{
    nvs_handle handle;
#if DECODER_TYPE == DEVTYPE_SWITCH
    esp_err_t err = nvs_open("switch", NVS_READWRITE, &handle);
#endif
#if DECODER_TYPE == DEVTYPE_SIGNAL
    esp_err_t err = nvs_open("signal", NVS_READWRITE, &handle);
#endif
#if DECODER_TYPE == DEVTYPE_POWER
    esp_err_t err = nvs_open("power", NVS_READWRITE, &handle);
#endif
#if DECODER_TYPE == DEVTYPE_RM
    esp_err_t err = nvs_open("feedback", NVS_READWRITE, &handle);
#endif
    if (err == ESP_OK)
    {
        nvs_erase_all(handle); // <-- löscht ALLE Keys in diesem Namespace
        nvs_commit(handle);
        nvs_close(handle);
#if DECODER_TYPE == DEVTYPE_SWITCH
        Serial.println("NVS: Namespace 'switch' gelöscht.");
#endif
#if DECODER_TYPE == DEVTYPE_SIGNAL
        Serial.println("NVS: Namespace 'signal' gelöscht.");
#endif
#if DECODER_TYPE == DEVTYPE_POWER
        Serial.println("NVS: Namespace 'power' gelöscht.");
#endif
#if DECODER_TYPE == DEVTYPE_RM
        Serial.println("NVS: Namespace 'feedback' gelöscht.");
#endif
    }
    else
    {
        Serial.printf("NVS open failed: %s\n", esp_err_to_name(err));
    }
}

// ID_ASSIGN (MAC‑basiert) im Decoder
// Das ist der Spezialfall für MAC_DIRECT, da hier die Ziel‑MAC im Paket steht und geprüft werden muss, ob sie mit der eigenen MAC übereinstimmt.
bool handleMacDirectFrame(uint8_t featureId, uint8_t commandId, const uint8_t *data, int len)
{
    if (featureId != FEATURE_ALIVE || commandId != FRAME_DECODER_ID_ASSIGN)
        return false;
    if (wrong_nextType(commandId))
        return true;

    nextType = FRAME_DECODER_ALIVE_ACK;

    if (len < 1 + 1 + 6) // assignedId + decoderType + MAC
        return false;

    uint8_t newAssignedId = data[0];
    uint8_t newDecoderType = data[1];
    const uint8_t *targetMac = &data[2];
    // 1. Nur Decoder ohne gültige ID reagieren
    if (aliveGetDecoderID() != INVALID_ASSIGNED_ID)
        return false; // Ich habe schon eine ID → ignorieren
    // 2. MAC-Vergleich: Bin ich der gemeinte Decoder?
    if (memcmp(targetMac, Decoder_Mac, 6) != 0)
        return false; // Nicht meine MAC → ignorieren
    // 3. Jetzt bin ich sicher: Ich bin gemeint
    aliveSetDecoderID(newAssignedId);
    DEVTYPE = newDecoderType;

    prefs.putByte(key_id, INVALID_ASSIGNED_ID); // erstmal ungültige ID speichern, damit bei einem Reboot die Hello-Sequenz wieder von vorne beginnt und nicht mit der alten ID gesendet wird
    prefs.putBool(key_otaPending, false);

    const uint8_t len_payload = 1;
    uint8_t payload[len_payload];
    payload[0] = aliveGetDecoderID();
    if (!otaInProgress)
        gDecoder->sendToBridge(FEATURE_ALIVE, FRAME_DECODER_ID_ACK, payload, len_payload);

    if (afterstartCallback != nullptr)
    {
        generateHash();
        afterstartCallback(); // decoderspezifische Aktionen
        delay(50);            // kurze Sicherheitspause
    }
    return true;
}

// Globale Kommandos (Reset to all, etc.)
bool handleGlobalFrame(uint8_t featureId, uint8_t commandId, const uint8_t *data, int len)
{
    if (featureId == FEATURE_DECODER)
    {
        switch (commandId)
        {
        case MSG_START_DECODERS:
            nextType = FRAME_DECODER_ID_ASSIGN;
            startAllowed = true;
            startHelloSequence();
            return true;
        case CMD_DECODER_RESET:
            if (beforeRestartCallback != nullptr)
            {
                beforeRestartCallback(); // decoderspezifische Aktionen
                delay(50);               // kurze Sicherheitspause
            }
            prefs.putByte(key_id, INVALID_ASSIGNED_ID);
            coreConfigClearOtaPending();
            delay(500); // kurze Pause, damit die Antwort auf die Reset-Anfrage noch rausgehen kann, bevor der Decoder neu startet
            ESP.restart();
            return true;
        }
    }

    if (featureId == FEATURE_BRIDGE2DECODER)
    {
        switch (commandId)
        {
        case PING:
            sendPING();
            return true;
        }
    }

    // Weitere globale Kommandos hier
    // Alle anderen CAN-Packets an ALLE Decoder weiterreichen
    gDecoder->onFeatureFrame(featureId, commandId, data, len);
    return false;
}

// Typ‑gebundene Kommandos (z.B. alle PowerDecoder sollen ihren MaxCurrent melden)
bool handleTypedFrame(uint8_t featureId, uint8_t commandId, const uint8_t *data, int len)
{
    // Hier nur noch inhaltliche Logik, kein „bin ich gemeint?“ mehr
    // hier wird onFeatureFrame nur vom ausgewählten Typ (zB switch) aufgerufen
    // die Auswahl eines bestimmten Decoders wird anhand der ID vorgenommen
    gDecoder->onFeatureFrame(featureId, commandId, data, len);
    return true;
}

// AssignedId‑gebundene Kommandos
bool handleIdBoundFrame(uint8_t featureId, uint8_t commandId, const uint8_t *data, int len)
{
    /*        FEATURE_ALIVE, FRAME_DECODER_ID_ASSIGN,
    FEATURE_brigde2Decoder
    EATURE_SETTING, SETTING_CMD_RESET_TO_DEFAULTS*/

    // Hier z.B. GET_SETTINGS, SET_SETTINGS, etc.
    if (featureId == FEATURE_SETTING && commandId == SETTING_CMD_RESET_TO_DEFAULTS)
    {
        DecoderReset::performReset(
            DECODER_NVS_NAMESPACE,   // NVS‑Namespace
            loadDefaultsWithAddress, // Callback
            FEATURE_SETTING,         // Feature für ACK
            SETTING_CMD_RESET_ACK,   // ACK‑Kommando
            aliveGetDecoderID(),
            DECODER_TYPE);
    }
    if (featureId == FEATURE_ALIVE && commandId == FRAME_DECODER_ALIVE_ACK)
    {
        aliveHandleAck(data, len);
    }

    if (featureId == FEATURE_OTA && commandId == OTA_START_WIFI)
    {
        if (len < 2)
            return true;
        otaRequested = true;
        otaRunning = true;

        esp_now_deinit();
        espNowReady = false;
        return true;
    }
    
    if (featureId == FEATURE_BRIDGE2DECODER && commandId == CONFIG_Status)
    {
        sendConfig();
        return true;
    }

    gDecoder->onFeatureFrame(featureId, commandId, data, len);

    return true;
}
