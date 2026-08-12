#include <Arduino.h>
#include <esp_now.h>
#include <WiFiUdp.h>
#include "protocol_constants_core.h"
#include "globals.h"
#include "CAN_proc.h"
#include "gui_proc.h"
#include "Bridge_Decoder.h"
#include "alive.h"
#include "PacketHandler.h"
#include "MOD-LCD.h"

// Alive / Reset
unsigned long lastAliveSent = 0;
const unsigned long ALIVE_TIMEOUT_MS = 5000;

// Gloabale Liste für ausstehende Assigns
std::vector<PendingAssign> pendingAssigns;

uint16_t CountDecoders;

// ----------------------------------------------------
// InitAlive
// ----------------------------------------------------
void InitAlive()
{
    resetFlag = true; // nach Neustart: GUI muss Reset-Ack schicken
    lastGuiSeen = millis();
    lastAliveSent = 0;
    lastPingSend = 0;
    lastRequest = 0;
    lastGuiAlive = 0;
    guiIsAlive = false;
    CountDecoders = 0;
}

// ----------------------------------------------------
// Alive-Überwachung
// ----------------------------------------------------
void checkAliveStates()
{
    if (GUIipIsNotSet())
        return;

    for (size_t i = 0; i < decoders.size(); i++)
    {
        DecoderInfo d = decoders[i]; // Kopie
        if (millis() - d.lastSeen > ALIVE_INTERVAL)
        {
            /*Serial.printf("checkAliveStates timeout: MAC=%02X:%02X:%02X:%02X:%02X:%02X, assignedId=%d, type=%02X\r\n",
                          d.Decoder_Mac[0], d.Decoder_Mac[1], d.Decoder_Mac[2],
                          d.Decoder_Mac[3], d.Decoder_Mac[4], d.Decoder_Mac[5],
                          d.assignedId, d.decoderType);*/
            d.isReady = false;
        }
    }
}

// ----------------------------------------------------
// Alive-Paket an GUI senden (Bridge-Alive / Summary)
// FEATURE_ALIVE / HANDLE_ALIVE
// payload: [powerCount, switchCount, signalCount, feedbackCount]
// ----------------------------------------------------

void sendAliveSummary()
{
    int power = 0;
    int sw = 0;
    int signal = 0;
    int feedback = 0;
    uint16_t CountDecoders_old = CountDecoders;

    if (GUIipIsNotSet())
        return;

    for (size_t i = 0; i < decoders.size(); i++)
    {
        DecoderInfo d = decoders[i]; // Kopie

        if (!d.isReady)
            continue;

        switch (d.decoderType)
        {
        case DEVTYPE_POWER:
            power++;
            break;
        case DEVTYPE_SWITCH:
            sw++;
            break;
        case DEVTYPE_SIGNAL:
            signal++;
            break;
        case DEVTYPE_RM:
            feedback++;
            break;
        }
    }
    CountDecoders = power + sw + signal + feedback;
    if (CountDecoders > CountDecoders_old)
    {
        drawAliveCounts(power, sw, signal, feedback);
    }
    if (CountDecoders < CountDecoders_old)
    {
        displayLCD("Decoder count decreased");
    }
    std::vector<uint8_t> payload;
    payload.push_back((uint8_t)power);
    payload.push_back((uint8_t)sw);
    payload.push_back((uint8_t)signal);
    payload.push_back((uint8_t)feedback);

    auto packet = buildPacket(
        FEATURE_ALIVE,
        HANDLE_ALIVE,
        payload.data(),
        (uint8_t)payload.size());

    udpSendToGui(packet.data(), packet.size());
}

// ----------------------------------------------------
void handleAlivePacket(const uint8_t *mac,
                       const uint8_t *payload,
                       uint8_t len)
{
    if (len < 9)
        return;

    int index = findDecoderByMac(mac);
    if (index == -1)
    {
        Serial.println("WARNUNG: Alive von unbekanntem Decoder!");
        return;
    }

    // Direkter Zugriff – std::vector gibt eine echte Referenz zurück
    DecoderInfo &info = decoders[index];
    /*    payload[0] = decoderId;
        payload[1] = DEVTYPE;
        payload[2] = status;
        payload[3] = VERS_HIGH;
        payload[4] = VERS_LOW;
        memcpy(&payload[5], featureBuf, featureLen);
    */
    info.assignedId = payload[0];
    info.decoderType = payload[1];
    info.isReady = (payload[2] != 0);
    info.fwMajor = payload[3];
    info.fwMinor = payload[4];
    info.value0 = payload[5];
    info.value1 = payload[6];
    info.lastSeen = millis();

    // Alive-Detail an GUI
    std::vector<uint8_t> guiPayload(payload, payload + len);
    auto pkt = buildPacket(FEATURE_ALIVE,
                           FRAME_DECODER_ALIVE,
                           guiPayload.data(),
                           (uint8_t)guiPayload.size());
    udpSendToGui(pkt.data(), pkt.size());
}

// ----------------------------------------------------
// Alive-Timeouts prüfen
// ----------------------------------------------------
void checkAliveTimeouts()
{
    unsigned long now = millis();

    for (size_t i = 0; i < decoders.size(); i++)
    {
        DecoderInfo &d = decoders[i]; // Referenz
        if (d.isReady && (now - d.lastSeen > ALIVE_TIMEOUT_MS))
        {
            d.isReady = false; // Decoder gilt als ausgefallen
        }
    }
}

void processAssignRetries()
{
    uint32_t now = millis();

    for (auto &pa : pendingAssigns)
    {
        if (!pa.waitingForAck)
            continue;

        if (now < pa.nextRetryMillis)
            continue;

        uint8_t payload[8];
        payload[0] = pa.assignedId;
        payload[1] = pa.decoderType;
        memcpy(&payload[2], pa.mac, 6);
        auto pkt = buildPacket(
            FEATURE_ALIVE,
            FRAME_DECODER_ID_ASSIGN,
            payload,
            sizeof(payload));

        // Unicast senden
        // Paket erweitern um Mode + Selector
        uint8_t buffer[250];
        buffer[0] = (uint8_t)SendMode::MAC_DIRECT;
        buffer[1] = 0; // Typ, AssignedId oder 0
        memcpy(&buffer[2], pkt.data(), pkt.size());

        esp_now_send(pa.mac, buffer, pkt.size() + 2);
        //        esp_now_send(pa.mac, pkt.data(), pkt.size());
        // Broadcast senden
        //    sendTheData(SendMode::MAC_DIRECT, 0, pkt.data(), pkt.size());

        pa.retriesLeft--;
        pa.nextRetryMillis = now + 40;

        if (pa.retriesLeft == 0)
        {
            Serial.println("BRIDGE: ID_ASSIGN failed after retries");
            pa.waitingForAck = false;
        }
    }
}

// ----------------------------------------------------
// ID an Decoder senden (altes Hello/ID-Protokoll)
// ----------------------------------------------------
void bridgeSendAssignWithRetry(const uint8_t *mac, uint8_t assignedId, uint8_t decoderType)
{
    PendingAssign pa;
    memcpy(pa.mac, mac, 6);
    pa.assignedId = assignedId;
    pa.decoderType = decoderType;
    pa.retriesLeft = 5;            // Anzahl Wiederholungen
    pa.nextRetryMillis = millis(); // sofort senden
    pa.waitingForAck = true;

    pendingAssigns.push_back(pa);

    // unicast!
    esp_now_peer_info_t peer{};
    memcpy(peer.peer_addr, mac, 6);
    peer.channel = BRIDGE_AP_CHANNEL;
    peer.encrypt = false;

    esp_now_add_peer(&peer);
}

// ----------------------------------------------------
// Decoder in GUI bekannt machen
// ----------------------------------------------------
void sendDecoderAddedToGUI(const DecoderInfo &info)
{
    uint8_t payload[10];
    payload[0] = info.assignedId;
    payload[1] = info.decoderType;
    memcpy(&payload[2], info.Decoder_Mac, 6);
    payload[8] = info.value0;
    payload[9] = info.value1;

    auto packet = buildPacket(
        FEATURE_DECODER,
        FRAME_DECODER_ADDED,
        payload,
        sizeof(payload));

    udpSendToGui(packet.data(), packet.size());
}

// ----------------------------------------------------
// HELLO vom Decoder
// ----------------------------------------------------
void handleHelloPacket(const uint8_t *mac,
                       const uint8_t *payload,
                       uint8_t len)
{
    if (GUIipIsNotSet())
        return;

    int index = findDecoderByMac(mac);

    if (index == -1)
    {
        index = addDecoder(mac, payload);
    }
    else
    {
        Serial.printf("Decoder already registered. ID=%d\n\r", decoders[index].assignedId);
        DecoderInfo &info = decoders[index];
        info.lastSeen = millis();
    }
    bridgeSendAssignWithRetry(decoders[index].Decoder_Mac, decoders[index].assignedId, decoders[index].decoderType);
    Serial.printf("Found decoder with assignedId=%d, type=%02X\n\r", decoders[index].assignedId, decoders[index].decoderType);
}

// ----------------------------------------------------
// READY vom Decoder
// ----------------------------------------------------
void cleanupFinishedAssigns()
{
    pendingAssigns.erase(
        std::remove_if(
            pendingAssigns.begin(),
            pendingAssigns.end(),
            [](const PendingAssign &pa)
            { return !pa.waitingForAck; }),
        pendingAssigns.end());
}

void handleIdAssignAck(const uint8_t *mac, uint8_t assignedId)
{
    for (auto &pa : pendingAssigns)
    {
        if (!pa.waitingForAck)
            continue;

        if (memcmp(pa.mac, mac, 6) == 0 && pa.assignedId == assignedId)
        {
            esp_now_del_peer(pa.mac);
            int index = findDecoderByMac(pa.mac);
            if (index != -1)
                sendDecoderAddedToGUI(decoders[index]);
            pa.waitingForAck = false;
            return;
        }
    }
}

void handleReadyPacket(const uint8_t *mac,
                       const uint8_t *payload,
                       uint8_t len)
{
    (void)payload;
    (void)len;

    int index = findDecoderByMac(mac);
    if (index == -1)
        return;

    DecoderInfo info = decoders[index];
    handleIdAssignAck(mac, info.assignedId);
    info.isReady = true;
    info.lastSeen = millis();
    decoders[index] = info;
}

// ----------------------------------------------------
// Power vom Decoder (altes AlivePacket-Format)
// ----------------------------------------------------
// payload: [decoderId, decoderType, status, fwMajor, fwMinor, current01, current02, current11, current12]

void handlePowerPacket(const uint8_t *mac,
                       uint8_t commandId,
                       const uint8_t *payload,
                       uint8_t payloadLen)
{
    (void)mac;

    // Optional: DecoderInfo aktualisieren, z.B. bei Shutdown/Threshold
    auto pkt = buildPacket(
        FEATURE_POWER,
        commandId,
        payload,
        payloadLen);
    sendPacketToGUI(pkt);
}

void handleSettingPacket(const uint8_t *mac,
                         uint8_t commandId,
                         const uint8_t *payload,
                         uint8_t payloadLen)
{
    (void)mac;

    auto pkt = buildPacket(
        FEATURE_SETTING,
        commandId,
        payload,
        payloadLen);
    sendPacketToGUI(pkt);
}

void handleSwitchSignalPacket(const uint8_t *mac,
                              uint8_t featureId,
                              uint8_t commandId,
                              const uint8_t *payload,
                              uint8_t payloadLen)
{
    (void)mac;
    // 1. Payload in sicheren lokalen Buffer kopieren
    uint8_t safePayload[32];
    uint8_t safeLen = min(payloadLen, (uint8_t)sizeof(safePayload));
    memcpy(safePayload, payload, safeLen);

    // 2. Erwartete Länge bestimmen
    uint8_t expectedLen = 0;

    switch (commandId)
    {
    case SWITCH_CMD_SET_DIR:
    case SIGNAL_CMD_SET_DIR:
    case SWITCH_CMD_SET_END_ACK:
    case SIGNAL_CMD_SET_END_ACK:
        expectedLen = 2;
        break;

    case SWITCH_CMD_GET_SETTINGS:
        expectedLen = EXP_LNG_SWITCH_SETTINGS;
        break;

    case SIGNAL_CMD_GET_SETTINGS:
        expectedLen = EXP_LNG_SIGNAL_SETTINGS;
        break;

    default:
        Serial.printf("Unknown commandId=%u\r\n", commandId);
        return;
    }

    // 3. Länge prüfen
    if (safeLen != expectedLen)
    {
        Serial.printf("Invalid payloadLen=%u (expected %u) - Command: %02X\r\n",
                      safeLen, expectedLen, commandId);
        return;
    }

    // 4. Sicheres Paket bauen
    auto pkt = buildPacket(
        featureId,
        commandId,
        safePayload,
        expectedLen);

    // 5. Sicher senden
    sendPacketToGUI(pkt);
}
