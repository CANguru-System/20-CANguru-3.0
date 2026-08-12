#include <Arduino.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <Update.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "Core_Ota.h"
#include "Core_EspNow.h"
#include "protocol_constants_core.h"
#include "Core_Config.h"
#include "Core_Logging.h"
#include "Core_globals.h"
#include "DecoderBase.h"

volatile bool otaRequested = false;
bool otaInProgress = false;
bool otaRunning = false;

const char *OTA_URL = "http://192.168.4.1/firmware.bin";
const char *OTA_SSID = "BridgeOTA";
const char *OTA_PASS = "";

static bool connectToBridgeAP()
{
    WiFi.mode(WIFI_MODE_STA);
    WiFi.persistent(false);

    // Sleep AUS – sonst fliegt er bei großen Transfers gerne raus
    WiFi.setSleep(false);

    WiFi.begin(OTA_SSID, OTA_PASS);

    Serial.print("DECODER: Verbinde mit AP ");
    Serial.println(OTA_SSID);

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 15000)
    {
        delay(250);
        Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.print("DECODER: Verbunden, IP: ");
        Serial.println(WiFi.localIP());
        return true;
    }
    else
    {
        Serial.println("DECODER: WiFi-Verbindung fehlgeschlagen");
        return false;
    }
}

void sendOtaError(uint8_t errorCode)
{
    const uint8_t len = 2;
    uint8_t pkt[len];
    pkt[0] = aliveGetDecoderID();
    pkt[1] = errorCode;

    if (!otaInProgress)
        gDecoder->sendToBridge(FEATURE_OTA, OTA_ERROR, pkt, len);
}

static void finishOtaFailure()
{
    Serial.println("OTA: Failure, restoring ESP-NOW");

    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    delay(150);

    WiFi.mode(WIFI_STA);
    esp_err_t r = esp_now_init();
    if (r != ESP_OK)
        Serial.printf("DECODER in finishOtaFailure: ESP-NOW init failed! err=%d\r\n", r);

    otaInProgress = false;
    otaRunning = false;
}


void sendOtaErrorFrame()
{
    const uint8_t len = 1;
    uint8_t pkt[len];
    pkt[0] = aliveGetDecoderID();

    gDecoder->sendToBridge(FEATURE_OTA, OTA_CURRENT_ERROR, pkt, len);
}

bool coreOtaStartFromBridge() {

    if (!connectToBridgeAP()) {
        Serial.println("DECODER: OTA abgebrochen (keine AP-Verbindung)");
        return false;
    }

    esp_now_deinit();
    espNowReady = false;

    WiFiClient client;
    HTTPClient http;

    if (!http.begin(client, OTA_URL)) {
        Serial.println("DECODER: HTTP begin fehlgeschlagen");
        return false;
    }

    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        Serial.print("DECODER: HTTP GET fehlgeschlagen, Code: ");
        Serial.println(httpCode);
        http.end();
        return false;
    }

    int contentLength = http.getSize();
    if (contentLength <= 0) {
        Serial.println("DECODER: Ungültige Content-Length");
        http.end();
        return false;
    }

    if (!Update.begin(contentLength)) {
        Serial.println("DECODER: Update.begin() fehlgeschlagen");
        http.end();
        return false;
    }

    WiFiClient* stream = http.getStreamPtr();
    size_t written = 0;
    uint8_t buff[BlockSizeDecoder];

    otaInProgress = true;

    while (http.connected() && written < (size_t)contentLength) {
        size_t available = stream->available();
        if (available) {
            int toRead = available;
            if (toRead > (int)sizeof(buff))
                toRead = sizeof(buff);
            int read = stream->readBytes(buff, toRead);
            if (read > 0) {
                size_t w = Update.write(buff, read);
                written += w;
            //    Serial.print(".");
            }
        }
        delay(1);
    }
    Serial.printf("DECODER: Firmware empfangen, Größe: %d bytes\r\n", written);
    Serial.println();

    coreConfigSetOtaPending(true);

    if (!Update.end()) {
        Serial.print("DECODER: Update.end() Fehler: ");
        Serial.println(Update.getError());
        http.end();
        finishOtaFailure();
        return false;
    }

    if (!Update.isFinished()) {
        Serial.println("DECODER: Update nicht vollständig");
        http.end();
        finishOtaFailure();
        return false;
    }

    Serial.println("DECODER: OTA erfolgreich, starte neu...");
    http.end();
    prefs.putByte(key_id, aliveGetDecoderID());
    delay(1000);
    ESP.restart();
    return true;
}

void coreOtaInit()
{
    // Flags sind bereits in coreEspNowInit zurückgesetzt
}

void coreOtaLoop()
{
    if (otaRequested && !otaInProgress)
    {
        otaRequested = false;
        coreOtaStartFromBridge();
    }
}
