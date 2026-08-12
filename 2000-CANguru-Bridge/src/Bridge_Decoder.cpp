#include <Arduino.h>
#include <esp_wifi.h>
#include <esp_now.h>
#include "protocol_constants_core.h"
#include "globals.h"
#include "alive.h"
#include "Bridge_Decoder.h"
#include "Bridge_OTA.h"
#include "otaQueue.h"
#include "gui_proc.h"
#include "CAN_proc.h"
#include "wdp_proc.h"
#include "handlers.h"

bool otaRunning = false;

const char *OTA_SSID = "BridgeOTA";
const char *OTA_PASS = "";

// die zentrale Sendefunktion
void sendTheData(SendMode mode, uint8_t selector, const uint8_t *data, size_t len)
{
    /*
    Protokoll der gesendeten Daten:
    [0] SendMode
    [1] Selector
    [2] Feature
    [3] Frame
    [4..] Payload
*/
    /*enum class SendMode : uint8_t {
        GLOBAL,         // an alle Decoder
        TYPE,           // an alle Decoder eines Typs
        ASSIGNED_ID,    // an genau einen Decoder
        MAC_DIRECT      // nur für ID_ASSIGN
    };
    */

    // Paket erweitern um Mode + Selector
    uint8_t buffer[250];
    buffer[0] = (uint8_t)mode;
    buffer[1] = selector; // Typ, AssignedId oder 0
    memcpy(&buffer[2], data, len);

    esp_now_send(Bridge_Mac, buffer, len + 2);
}

void onBridgeDataSent(const uint8_t *mac, esp_now_send_status_t status)
{
    //  Serial.printf("BRIDGE: send to %02X:%02X:%02X:%02X:%02X:%02X status=%d\r\n",
    //                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], status);
}

void handleOtaPacket(const uint8_t *mac, uint8_t commandId, const uint8_t *payload, int payloadLen)
{
    switch (commandId)
    {
    case OTA_FW_ACK:
        otaHandleDecoderAck(mac, payload, payloadLen);
        return;

    case /* The code seems to be a comment in C++ indicating that the Over-The-Air (OTA) firmware
    update is done. */
        OTA_FW_DONE:
        otaHandleDecoderDone(mac, payload, payloadLen);
        return;

    case OTA_SUCCESS:
        Serial.println("OTA_SUCCESS received from MAC");
        //        stopBridgeOtaApMode();
        //        restoreBridgeEspNow();
        // KEIN MSG_START_DECODERS !!!
        return;

    case OTA_ERROR:
        Serial.println("OTA_ERROR received from MAC");
        //        stopBridgeOtaApMode();
        //        restoreBridgeEspNow();
        return;
    }
}

void handlePowerPacket(const uint8_t *mac, uint8_t commandId, const uint8_t *payload, int payloadLen)
{
    ParsedPacket p;
    p.featureId = FEATURE_POWER;

    switch (commandId)
    {
    case POWER_CMD_SET_THRESHOLD:
    {
        p.commandId = POWER_CMD_SET_THRESHOLD;
        p.payload = {payload[0], payload[1]};
        p.valid = true;
        sendPacketToGUI(p);
        return;
    }

    case POWER_CMD_GET_THRESHOLD:
    {
        p.commandId = POWER_CMD_GET_THRESHOLD;
        p.payload = {payload[0], payload[1]};
        p.valid = true;

        sendPacketToGUI(p);
        return;
    }

    case POWER_CMD_GET_VERSION:
    {
        p.commandId = POWER_CMD_GET_VERSION;
        p.payload = {payload[0], payload[1]};
        p.valid = true;

        sendPacketToGUI(p);
        return;
    }
    }
}
void sendGuiFeedbackSettings(const uint8_t *values)
{
    ParsedPacket p;

    p.featureId = FEATURE_FEEDBACK;
    p.commandId = FEEDBACK_CMD_GET_SETTINGS;

    // 16 Bytes Payload übernehmen
    p.payload.assign(values, values + 16);
    //         memcpy(p.payload.data(), payload, 16);
    p.valid = true;

    sendPacketToGUI(p);
}

void sendGuiFeedbackAck()
{
    ParsedPacket p;
    p.featureId = FEATURE_FEEDBACK;
    p.commandId = FEEDBACK_CMD_SET_SETTINGS_ACK;
    p.valid = true;

    sendPacketToGUI(p);
}

void handleFeedbackDecoderResponse(uint8_t cmd, const uint8_t *payload, int len)
{
    Serial.printf("Received Feedback response from decoder: cmd=%u, len=%d\r\n", cmd, len);
    if (cmd == FEEDBACK_CMD_GET_SETTINGS)
    {
        if (len != 16)
        {
            Serial.println("Feedback: ungültige Länge!");
            return;
        }

        sendGuiFeedbackSettings(payload);
    }
    if (cmd == FEEDBACK_CMD_SET_SETTINGS_ACK)
    {
        sendGuiFeedbackAck();
        return;
    }
}

void onDecoderReceivedPacketFromDecoder(const uint8_t *mac,
                                        uint8_t featureId,
                                        uint8_t commandId,
                                        const uint8_t *payload,
                                        int payloadLen)
{
    // -----------------------------------
    // FEATURE_DECODER (HELLO, READY) 0xD0
    // ------------------------------------
    if (featureId == FEATURE_DECODER)
    {
        switch (commandId)
        {
        case MSG_START_DECODERS:
            // GUI → Bridge → Decoder
            // einfach an Decoder weiterleiten
            sendTheData(SendMode::GLOBAL, 0, payload, payloadLen);
            return;
        case FRAME_DECODER_REJOIN:
            uint8_t id = payload[0];

            for (size_t i = 0; i < decoders.size(); i++)
            {
                DecoderInfo &d = decoders[i];
                if (d.assignedId == id)
                {
                    Serial.printf("Decoder with ID %d has REJOINED\r\n", id);
                    otaRunning = false;
                    d.isReady = true;
                    otaSuccessReceived = true;
                    break;
                }
            }
        }
    }

    // -----------------------------------
    // FEATURE_CS2 eingepackte CAN-Frames 0x80
    // ------------------------------------
    if (featureId == FEATURE_CS2)
    {
        switch (commandId)
        {
        case FRAME_CAN_2BRIDGE:
            // GUI → Bridge → Decoder
            // einfach an GUI und WDP weiterleiten
            sendToWDP(payload);
            enqueueCanToFifo(payload, 'D');
            return;
        }
    }

    // -------------------------------
    // FEATURE_ALIVE 0x70
    // -------------------------------
    if (featureId == FEATURE_ALIVE)
    {
        switch (commandId)
        {
        case FRAME_DECODER_ALIVE:
            handleAlivePacket(mac, payload, payloadLen);
            return;

        case FRAME_DECODER_HELLO:
            handleHelloPacket(mac, payload, payloadLen);
            return;

        case FRAME_DECODER_ID_ACK:
            handleReadyPacket(mac, payload, payloadLen);
            return;

        case FRAME_DECODER_ALIVE_ACK:
            // optional: Bridge braucht ACK nicht
            return;
        }
    }

    // -------------------------------
    // FEATURE_POWER 0x20
    // -------------------------------
    if (featureId == FEATURE_POWER)
    {
        handlePowerPacket(mac, commandId, payload, payloadLen);
        return;
    }
    // -------------------------------
    // FEATURE_FEEDBACK 0x50
    // -------------------------------
    if (featureId == FEATURE_FEEDBACK)
    {
        handleFeedbackDecoderResponse(commandId, payload, payloadLen);
        return;
    }

    // -------------------------------
    // FEATURE_SWITCH 0x30
    // -------------------------------
    if (featureId == FEATURE_SWITCH || featureId == FEATURE_SIGNAL)
    {
        handleSwitchSignalPacket(mac, featureId, commandId, payload, payloadLen);
        return;
    }

    // -------------------------------
    // FEATURE_SETTING 0xB0
    // -------------------------------
    if (featureId == FEATURE_SETTING)
    {
        handleSettingPacket(mac, commandId, payload, payloadLen);
        return;
    }

    // -------------------------------
    // FEATURE_OTA 0xC0
    // -------------------------------
    if (featureId == FEATURE_OTA)
    {
        Serial.println("Received OTA packet, handling it...");
        handleOtaPacket(mac, commandId, payload, payloadLen);
        return;
    }

    // -------------------------------
    // Restliche Pakete → an GUI
    // -------------------------------
    auto bytes = buildPacket(
        featureId,
        commandId,
        payload,
        payloadLen);
    sendPacketToGUI(bytes);
    return;
}

// Eingehende Nachrichten von den Decodern (über ESP-NOW) werden hier empfangen und verarbeitet. Je nach FeatureID und CommandID werden sie an die entsprechenden Handler weitergeleitet oder an die GUI gesendet.
static void onBridgeDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
    // ------------------------------------------------------------
    // 1) Feature | Frame | Payload
    // ------------------------------------------------------------
    if (len < 2)
    {
        return;
    }

    uint8_t featureID = incomingData[0];
    uint8_t commandID = incomingData[1];
    const uint8_t *payload = nullptr;
    int payloadLen = len - 2;

    if (payloadLen > 0)
        payload = &incomingData[2];

    // Weiterreichen an deine Bridge-Logik
    onDecoderReceivedPacketFromDecoder(mac, featureID, commandID, payload, payloadLen);
}

// Fehlermeldungen
void printESPNowError(esp_err_t Result)
{
    if (Result == ESP_ERR_ESPNOW_NOT_INIT)
    {
        // How did we get so far!!
        Serial.println("ESPNOW not Init.");
    }
    else if (Result == ESP_ERR_ESPNOW_ARG)
    {
        Serial.println("Invalid Argument");
    }
    else if (Result == ESP_ERR_ESPNOW_INTERNAL)
    {
        Serial.println("Internal Error");
    }
    else if (Result == ESP_ERR_ESPNOW_NO_MEM)
    {
        Serial.println("ESP_ERR_ESPNOW_NO_MEM");
    }
    else if (Result == ESP_ERR_ESPNOW_NOT_FOUND)
    {
        Serial.println("Peer not found!");
    }
    else if (Result == ESP_ERR_ESPNOW_IF)
    {
        Serial.println("Interface Error.");
    }
    else
    {
        int res = Result;
        char chs[30];
        sprintf(chs, "\r\nNot sure what happened\t%d", res);
        Serial.println(chs);
    }
}

bool findDecoderByAssignedId(uint8_t id, DecoderInfo &out)
{
    for (size_t i = 0; i < decoders.size(); i++)
    {
        DecoderInfo &d = decoders[i]; // Referenz

        if (d.assignedId == id)
        {
            out = d; // in out kopieren
            return true;
        }
    }
    return false;
}

bool findDecoderByHash(uint8_t hash, DecoderInfo &out)
{
    // Ermitteln der ID aus der Basis-Hash
    uint8_t id = hash - uid_device[3]; // data[8] enthält die Basis-Hash, die von der GUI gesendet wird

    return findDecoderByAssignedId(id, out);
}

bool findDecoderByDecoderType(uint8_t type, DecoderInfo &out)
{
    Serial.printf("Looking for Decoder Type: %u\r\n", type);
    for (size_t i = 0; i < decoders.size(); i++)
    {
        DecoderInfo &d = decoders[i]; // Referenz

        if (d.decoderType == type)
        {
            out = d; // in out kopieren
            Serial.printf("Found decoder with type %u\r\n", d.decoderType);
            return true;
        }
    }
    return false;
}
#include <WiFi.h>
#include "esp_wifi.h"

void initBridgeWifi()
{
    // 1. WiFi komplett initialisieren
    WiFi.mode(WIFI_MODE_APSTA); // falls du STA parallel nutzt, sonst WIFI_MODE_AP
    WiFi.persistent(false);

    // 2. SoftAP starten
    bool apOk = WiFi.softAP(OTA_SSID, OTA_PASS);
    if (!apOk)
    {
        Serial.println("BRIDGE: softAP start FAILED");
        return;
    }

    Serial.print("BRIDGE: AP IP: ");
    Serial.println(WiFi.softAPIP());

    // 3. Sleep AUS – zwingend für stabile OTA-Transfers
    WiFi.setSleep(false);

    // 4. Beacon-Intervall erhöhen (Standard 100 ms → 600 ms)
    wifi_config_t conf;
    esp_wifi_get_config(WIFI_IF_AP, &conf);
    conf.ap.beacon_interval = 600; // 600 ms
    esp_wifi_set_config(WIFI_IF_AP, &conf);

    // 5. Bandbreite auf HT40 (optional, aber gut)
    esp_wifi_set_bandwidth(WIFI_IF_AP, WIFI_BW_HT40);

    // 6. (Optional) TX-Power hochsetzen
    // esp_wifi_set_max_tx_power(78); // 78 = ~19.5 dBm

    Serial.println("BRIDGE: WiFi AP ready for decoders");
}

void initBridgeEspNow()
{

    // WLAN in definierten Zustand bringen
    WiFi.disconnect(true, true);
    delay(50);
    initBridgeWifi();
    /*    // Dauerhaft AP+STA
        WiFi.mode(WIFI_AP_STA);
        delay(50);

        // Power Save aus
        esp_wifi_set_ps(WIFI_PS_NONE);

        // Kanal fest auf 1
        esp_wifi_set_channel(BRIDGE_AP_CHANNEL, WIFI_SECOND_CHAN_NONE);
        delay(50);

        // AP starten (für OTA)
        bool apOk = WiFi.softAP(OTA_SSID, OTA_PASS, BRIDGE_AP_CHANNEL, 0);
    */
    // alten ESPNOW-Stack aufräumen
    esp_now_deinit();
    delay(20);

    // ESPNOW starten
    esp_err_t r = esp_now_init();
    if (r != ESP_OK)
    {
        Serial.printf("BRIDGE: esp_now_init FAILED: %d\r\n", r);
        return;
    }
    esp_now_peer_info_t peer{};
    memcpy(peer.peer_addr, Bridge_Mac, 6);
    peer.channel = BRIDGE_AP_CHANNEL;
    peer.ifidx = WIFI_IF_STA;
    peer.encrypt = false;

    r = esp_now_add_peer(&peer);
    if (r != ESP_OK)
        Serial.printf("BRIDGE: ESP-NOW ADD PEER ERROR: %s\n\r", esp_err_to_name(r));

    esp_now_register_recv_cb(onBridgeDataRecv);
    esp_now_register_send_cb(onBridgeDataSent);
}

void sendFrame2allDecoders(uint8_t *data, int len)
{
    uint8_t payload[CAN_FRAME_SIZE];
    memcpy(&payload, data, CAN_FRAME_SIZE);

    // Packet bauen
    auto pkt = buildPacket(FEATURE_BRIDGE2DECODER, data[CANcmd], payload, sizeof(payload));
    sendTheData(SendMode::GLOBAL, 0, pkt.data(), pkt.size());

    delay(50);
}

void sendFrame2TypeDecoders(uint8_t *data, int len, uint8_t decoderType)
{
    uint8_t payload[CAN_FRAME_SIZE];
    memcpy(&payload, data, CAN_FRAME_SIZE);

    // Packet bauen
    auto pkt = buildPacket(FEATURE_BRIDGE2DECODER, data[CANcmd], payload, sizeof(payload));
    sendTheData(SendMode::TYPE, decoderType, pkt.data(), pkt.size());
    delay(50);
}

void sendConfigData(const uint8_t *data)
{
    uint8_t payload[CAN_FRAME_SIZE];
    memcpy(&payload, data, CAN_FRAME_SIZE);
    // Packet bauen
    auto pkt = buildPacket(FEATURE_BRIDGE2DECODER, data[CANcmd], payload, sizeof(payload));
    DecoderInfo d;
    if (findDecoderByHash(data[8], d))
    {
        sendTheData(SendMode::ASSIGNED_ID, d.assignedId, pkt.data(), pkt.size());
    }
}

//-----------------------------------------
// Discovery-Response an GUI
//-----------------------------------------

void sendStartToDecoders()
{
    uint8_t pkt[1] = {0x00};
    auto bytes = buildPacket(FEATURE_DECODER, MSG_START_DECODERS, pkt, sizeof(pkt));
    sendTheData(SendMode::GLOBAL, 0, bytes.data(), bytes.size());
    Serial.println("Sent START_DECODERS to all decoders");
}

void sendResetToDecoders()
{
    uint8_t pkt[1] = {0x00};
    auto bytes = buildPacket(FEATURE_DECODER, CMD_DECODER_RESET, pkt, sizeof(pkt));
    sendTheData(SendMode::GLOBAL, 0, bytes.data(), bytes.size());
    delay(50);
}
