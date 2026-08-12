#include <Arduino.h>
#include <ETH.h>
#include "alive.h"
#include "can_proc.h"
#include "globals.h"
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "Bridge_Decoder.h"
#include "MOD-LCD.h"
#include "protocol_constants_core.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#define WDP_FRAME_SIZE 64 // oder was du brauchst

QueueHandle_t wdpQueue = NULL;

void initWdpQueue()
{
  wdpQueue = xQueueCreate(20, WDP_FRAME_SIZE);
  if (wdpQueue == NULL)
  {
    Serial.println("ERROR: Could not create WDP queue!");
  }
}

// -------------------- Senden an WDP -------------------

void wdpSendTask(void *pv)
{
  uint8_t frame[WDP_FRAME_SIZE];

  for (;;)
  {
    if (xQueueReceive(wdpQueue, frame, portMAX_DELAY))
    {
      // direkt über UDP senden
      udpWDP.beginPacket(wdpIP, WDP_PORT_OUT);
      udpWDP.write(frame, WDP_FRAME_SIZE);
      udpWDP.endPacket();

      // kleine Pause, um Bursts zu glätten
      vTaskDelay(5 / portTICK_PERIOD_MS);
    }
  }
}

void sendToWDP(const uint8_t *buffer)
{
  if (wdpQueue == NULL)
    return;

  xQueueSend(wdpQueue, buffer, portMAX_DELAY);
}

// -------------------- Empfangen von WDP -------------------

void readWDP()
{
  uint8_t UDPbuffer[CAN_FRAME_SIZE]; // buffer to hold incoming packet
  uint8_t CANbuffer[CAN_FRAME_SIZE]; // buffer to hold incoming packet
  uint8_t M_PING_RESPONSEx[] = {0x00, 0x30, 0x00, 0x00, 0x00};
  uint8_t CAN_PING_CS2[] = {0x00, 0x31, 0x47, 0x11, 0x08, 0x00, 0x00, 0x00, 0x00, 0x03, 0x08, 0xFF, 0xFF};
  uint8_t CAN_PING_CS2_1[] = {0x00, 0x31, 0x63, 0x4A, 0x08, 0x00, 0x00, 0x00, 0x00, 0x04, 0x02, 0xFF, 0xF0};
  uint8_t CAN_PING_CS2_2[] = {0x00, 0x31, 0x63, 0x4B, 0x08, 0x00, 0x00, 0x00, 0x00, 0x03, 0x44, 0x00, 0x00};
  int packetSize = udpWDP.parsePacket();
  // if there's data available, read a packet
  if (packetSize)
  {
    // IP von WDP übernehmen
    wdpIP = udpWDP.remoteIP();
    // read the packet into packetBufffer
    udpWDP.read(UDPbuffer, CAN_FRAME_SIZE);
    // send received data to CAN
    memcpy(&CANbuffer, &UDPbuffer, CAN_FRAME_SIZE);
    // Für Testzwecke hier die empfangenen Daten von WDP anzeigen
    //    Serial.print("From WDP:");
    //    log_buf_d(CANbuffer, CAN_FRAME_SIZE);
    // alles an die Gleisbox
    enqueueCANFrame2CAN(CANbuffer, hashDisabled);
    // alles an die GUI
    enqueueCanToFifo(CANbuffer, 'W');

    switch (CANbuffer[CANcmd])
    {
    case PING:
      sendToWDP(CAN_PING_CS2);
      sendFrame2allDecoders(CANbuffer, CAN_FRAME_SIZE);
      break;

    case PING_R:
      //    log_d("PING_R from WDP received");
      if ((CANbuffer[data6] == 0xEE) && (CANbuffer[data7] == 0xEE))
      {
        delay(wait_time_small);
        UDPbuffer[0x00] = 0x00;
        UDPbuffer[0x01] = PING;
        UDPbuffer[0x02] = 0x00;
        UDPbuffer[0x03] = 0x00;
        UDPbuffer[0x04] = 0x00;
        sendToWDP(UDPbuffer);
        delay(100);
        memcpy(&UDPbuffer, &CAN_PING_CS2_1, CAN_FRAME_SIZE);
        delay(50);
        memcpy(&UDPbuffer, &CAN_PING_CS2_2, CAN_FRAME_SIZE);
      }
      break;

    case CONFIG_Status:
      sendConfigData(CANbuffer);
      break;

    case SWITCH_ACC:
      sendFrame2TypeDecoders(CANbuffer, CAN_FRAME_SIZE, DEVTYPE_SWITCH);
      sendFrame2TypeDecoders(CANbuffer, CAN_FRAME_SIZE, DEVTYPE_SIGNAL);
      break;

    default:
      // fast alles an die Decoder
      sendFrame2allDecoders(CANbuffer, CAN_FRAME_SIZE);
      break;
    }
  }
}
// ----------------------------------------------------
// CS2-Download-Routen
// ----------------------------------------------------

void setupDownloadRoutesCS2()
{
  otaServer.on("/betatest/cs2/lokomotive.cs2", HTTP_GET,
               [](AsyncWebServerRequest *request)
               {
                 request->send(LittleFS, "/lokomotive.cs2", "text/plain");
               });

  otaServer.on("/config/lokomotive.cs2", HTTP_GET,
               [](AsyncWebServerRequest *request)
               {
                 request->send(LittleFS, "/lokomotive.cs2", "text/plain");
               });
}
// ----------------------------------------------------
// Upload-Route (GUI -> Bridge)
// ----------------------------------------------------

void setupUploadRoute()
{
  otaServer.on("/upload/lokomotive.cs2", HTTP_POST, [](AsyncWebServerRequest *request) {}, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
               {
              static File uploadFile;

              if (index == 0)
              {
                if (LittleFS.exists("/lokomotive.cs2"))
                  LittleFS.remove("/lokomotive.cs2");

                uploadFile = LittleFS.open("/lokomotive.cs2", "w");
              }

              uploadFile.write(data, len);

              if (final)
              {
                uploadFile.close();
                request->send(200, "text/plain", "OK");
              } });
}

void onRequest(AsyncWebServerRequest *request)
{
  Serial.println("onRequest");
  Serial.println(request->url());
}

// events
//*********************************************************************************************************
//  behandelt die diversen Stadien des ETHERNET-Aufbaus
//  Network event callback
void iNetEvtCB(arduino_event_id_t event, arduino_event_info_t info)
{
  switch (event)
  {
  case ARDUINO_EVENT_WIFI_SCAN_DONE:
    displayLCD("Scanning finished");
    break;
  case ARDUINO_EVENT_ETH_START: // SYSTEM_EVENT_ETH_START:
    displayLCD("ETHERNET Started");
    break;
  case ARDUINO_EVENT_ETH_CONNECTED: // SYSTEM_EVENT_ETH_CONNECTED:
    displayLCD("ETHERNET Connected");
    // set eth hostname here
    ETH.setHostname("CANguru-Bridge");
    break;
  case ARDUINO_EVENT_ETH_GOT_IP: // SYSTEM_EVENT_ETH_GOT_IP:
    displayIP(ETH.localIP());
    displayLCD("");
    displayLCD("Connect!");
    break;
  case ARDUINO_EVENT_ETH_DISCONNECTED: // SYSTEM_EVENT_ETH_DISCONNECTED:
    displayLCD("ETHERNET Disconnected");
    break;
  case ARDUINO_EVENT_ETH_STOP: // SYSTEM_EVENT_ETH_STOP:
    displayLCD("ETHERNET Stopped");
    break;
  default:
    log_i("ETHERNET ?? %X", event);
    break;
  }
}
