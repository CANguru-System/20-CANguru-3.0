#include <Arduino.h>
#include "protocol_constants_core.h"
#include "Bridge_Decoder.h"
#include <esp_now.h>
#include <CAN_const.h>
#include "driver/gpio.h"
#include "driver/twai.h"
#include <CAN_proc.h>
#include <gui_proc.h>
#include <alive.h>
#include <wdp_proc.h>
#include <globals.h>
#include <ETH.h>
#include <WiFiUdp.h>
#include "WiFiUdp.h"

ToGUICANFrameItem FiFoFromCAN[64];
volatile int FiFoFromCANHead = 0;
volatile int FiFoFromCANTail = 0;
uint32_t canSeq = 0;
ToCANItem FiFo2CAN[64];
volatile int ToCANHead = 0;
volatile int ToCANTail = 0;
uint8_t hasharr[2];
uint8_t uid_device[4];
bool GB_found;

void sendCanFrame(uint8_t *data, bool hash);

void generateHash()
{
#define UID_BASE 0x45009195ULL // CAN-UID
  //  uint32_t uid = UID_BASE + (DEVTYPE - DEVTYPE_BASE - 1) * maxdevice + offset;
  uint32_t uid = UID_BASE; // UID_BASE + offset;
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

// Optimierte TWAI‑Initialisierung und Loop‑Leselogik
// Initialisierung
void initTWAI()
{
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_5, GPIO_NUM_35, TWAI_MODE_NORMAL);
  g_config.rx_queue_len = 64;
  g_config.tx_queue_len = 32;

  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_250KBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK)
  {
    Serial.println("TWAI install failed");
    return;
  }

  if (twai_start() != ESP_OK)
  {
    Serial.println("TWAI start failed");
    return;
  }
  generateHash();
}

// -------------------- CAN-FIFO ------------------------
// FiFoFromCAN → geht zur GUI
// Das ist die FIFO für eingehende CAN‑Frames, also:
// Bridge empfängt CAN‑Frames vom CAN‑Bus
// legt sie in FiFoFromCAN
// sendFiFoFromCANToGUI() schickt sie per UDP an die GUI
// GUI sortiert sie und zeigt sie an
// 👉 FiFoFromCAN = CAN‑Empfangsrichtung

// ----------------------------------------------------
// CAN-Funktionen
// ----------------------------------------------------

// FiFoFromCAN = CAN rein
// eingehend

void enqueueCanToFifo(const uint8_t *buf, char c)
{
  int next = (FiFoFromCANHead + 1) % 64;
  if (next == FiFoFromCANTail)
  {
    // FIFO voll → verwerfen oder überschreiben
    return;
  }
  FiFoFromCAN[FiFoFromCANHead].seq = canSeq++;
  memcpy(FiFoFromCAN[FiFoFromCANHead].data, buf, CAN_FRAME_SIZE);
  FiFoFromCAN[FiFoFromCANHead].source = c;
  FiFoFromCANHead = next;
}

bool dequeueCanFromFifo(ToGUICANFrameItem &item)
{
  if (FiFoFromCANHead == FiFoFromCANTail)
    return false;
  item = FiFoFromCAN[FiFoFromCANTail];
  FiFoFromCANTail = (FiFoFromCANTail + 1) % 64;
  return true;
}

void readCANMsg()
{
  uint8_t buffer[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  twai_message_t msgReceived;
  char cntBuffer[125];

  while (twai_receive(&msgReceived, 0) == ESP_OK)
  {
    msgReceived.identifier &= CAN_EFF_MASK;
    msgReceived.identifier = htonl(msgReceived.identifier);
    memcpy(buffer, &msgReceived.identifier, 4);
    buffer[4] = msgReceived.data_length_code;
    memcpy(&buffer[5], msgReceived.data, msgReceived.data_length_code);
    // An die GUI
    enqueueCanToFifo(buffer, 'M');
    if (buffer[CANcmd]== PING_R && buffer[data7] == DEVTYPE_GB)
    {
      GB_found = true;
    }
  }
}

// FiFo2CAN = CAN raus
// ausgehend
// schreibt einen CAN-Frame in die ausgehende FIFO
void enqueueCANFrame2CAN(const uint8_t *data, bool hash)
{
  //  if (data[CANcmd] == 0x0F)
  //  {
  //    sendToWDP(data);
  //    return;
  //  }
  int next = (ToCANHead + 1) % 64;
  if (next == ToCANTail)
  {
    // FIFO voll → verwerfen oder überschreiben
    Serial.println("ToCAN CAN FIFO FULL");
    return;
  }

  memcpy(FiFo2CAN[ToCANHead].data, data, CAN_FRAME_SIZE);
  FiFo2CAN[ToCANHead].hash = hash;
  ToCANHead = next;
}

bool dequeueCANFrame2CAN(ToCANItem &item)
{
  if (ToCANHead == ToCANTail)
    return false;

  item = FiFo2CAN[ToCANTail];
  ToCANTail = (ToCANTail + 1) % 64;
  return true;
}

void sendCanFrame(uint8_t *data, bool hash)
{
  twai_message_t Message2Send;
  //
  // CAN uses (network) big endian format
  // Maerklin TCP/UDP Format: always 13 (CAN_FRAME_SIZE) bytes
  //   byte 0 - 3  CAN ID
  //   byte 4      DLC
  //   byte 5 - 12 CAN data
  //

  if (hash)
  {
    data[hash0] = hasharr[0];
    data[hash1] = hasharr[1];
  }

  // 1. CAN senden

  memset(&Message2Send, 0, CAN_FRAME_SIZE);
  Message2Send.rtr = CAN_MSG_FLAG_RTR_;
  Message2Send.ss = CAN_MSG_FLAG_SS_;
  Message2Send.extd = CAN_MSG_FLAG_EXTD_;
  memcpy(&Message2Send.identifier, data, 4);
  Message2Send.identifier = ntohl(Message2Send.identifier);
  // Anzahl Datenbytes
  Message2Send.data_length_code = data[4];
  // Datenbytes
  if (Message2Send.data_length_code > 0)
    memcpy(&Message2Send.data, &data[5], Message2Send.data_length_code);
  twai_transmit(&Message2Send, 0);
}

// ---------------------------------------------------------
// PowerDecoder: Change Track Current
// ---------------------------------------------------------

void sendPowerState(uint8_t state)
{
  uint8_t payload[1] = {state};

  auto pkt = buildPacket(
      FEATURE_POWER,
      POWER_CHG_TRACK_CURRENT,
      payload,
      sizeof(payload));

  udpSendToGui(pkt.data(), pkt.size());
  sendTheData(SendMode::TYPE, DEVTYPE_POWER, pkt.data(), pkt.size());
}

void processCANFrames2CAN()
{
  ToCANItem item;

  while (dequeueCANFrame2CAN(item))
  {

    // CAN-Frame senden
    sendCanFrame(item.data, item.hash);
    /*  SysSTOPP = new byte[]    { 0x00, 0x00, high, low, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        SysGO =    new byte[]    { 0x00, 0x00, high, low, 0x05, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00};
        magicStart0 = new byte[] { 0x00, 0x36, high, low, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, (byte)'G' };
        magicStart1 = new byte[] { 0x00, 0x36, high, low, 0x05, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, (byte)'G' };
    */
    // Stromanzeige auf der GUI aktualisieren
    if (item.data[Framelng] == 0x05 && item.data[CANcmd] == Sys_CMD)
    {
      sendPowerState(item.data[data4]);
    }
  }
}

// an die GUI senden
void sendFiFoFromCANToGUI()
{
  if (GUIipIsNotSet())
    return;

  ToGUICANFrameItem item;
  while (dequeueCanFromFifo(item))
  {
    //    if (item.data[0x00] == 0x0a)
    udp.beginPacket(guiIP, GUI_PORT);
    // zuerst die DSortiernummer
    udp.write((uint8_t *)&item.seq, sizeof(item.seq));
    // dann die Daten
    udp.write(item.data, CAN_FRAME_SIZE); // oder len, wenn du sie mitführst
    // zuletzt die Quelle
    udp.write((uint8_t *)&item.source, 1); // oder len, wenn du sie mitführst
    udp.endPacket();
    // An die WDP
    sendToWDP(item.data);
    //    printf("GUI: %d--%02X-%02X-%02X\r\n", item.seq, item.data[1], item.data[2], item.data[3]);
  }
}
