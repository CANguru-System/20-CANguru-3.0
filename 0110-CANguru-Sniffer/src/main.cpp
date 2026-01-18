#include <Arduino.h>

#include "Preferences.h"
#include "CANguruDefs.h"
#include <CAN_const.h>
#include "driver/twai.h"
#include <ETH.h>

unsigned long now_micros;

// In Millisekunden umrechnen
unsigned long total_ms = now_micros;

// Zerlegen
unsigned int minutes;
unsigned int seconds;
unsigned int ms;

// sendet den CAN-Frame buffer über den CAN-Bus an die Gleisbox
void proc2CAN(uint8_t *buffer)
{
  twai_message_t Message2Send;
  //
  // CAN uses (network) big endian format
  // Maerklin TCP/UDP Format: always 13 (CAN_FRAME_SIZE) bytes
  //   byte 0 - 3  CAN ID
  //   byte 4      DLC
  //   byte 5 - 12 CAN data
  //
  memset(&Message2Send, 0, CAN_FRAME_SIZE);
  Message2Send.rtr = CAN_MSG_FLAG_RTR_;
  Message2Send.ss = CAN_MSG_FLAG_SS_;
  Message2Send.extd = CAN_MSG_FLAG_EXTD_;
  memcpy(&Message2Send.identifier, buffer, 4);
  Message2Send.identifier = ntohl(Message2Send.identifier);
  // Anzahl Datenbytes
  Message2Send.data_length_code = buffer[4];
  // Datenbytes
  if (Message2Send.data_length_code > 0)
    memcpy(&Message2Send.data, &buffer[5], Message2Send.data_length_code);
  if (twai_transmit(&Message2Send, 3 * portTICK_PERIOD_MS) == ESP_FAIL)
    log_e("Failed to queue message for transmission\n");
}

void setup_can_driver()
{
  // start CAN Module
  // Initialize configuration structures using macro initializers
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_5, GPIO_NUM_35, TWAI_MODE_NORMAL);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_250KBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  // Install CAN driver
  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_FAIL)
    log_e("Failed to install driver\n");
  else
    Serial.println("\r\nDriver installed successfully");

  // Start CAN driver
  if (twai_start() == ESP_FAIL)
    log_e("Failed to start driver\n");
  else
    Serial.println("Driver started successfully\r\n");
}

void setup()
{
#if defined ARDUINO_ESP32_EVB
  delay(350);
#endif
  Serial.begin(bdrMonitor);
  delay(500);
  Serial.printf("\r\n\r\nC A N g u r u - S N I F F E R");
  Serial.printf("\r\n on %s", ARDUINO_BOARD);
  Serial.printf("\r\nCPU Frequency = %d Mhz", F_CPU / 1000000);
  //  log_e("ERROR!");
  //  Serial.printf("%02U:%02u:%03u - VERBOSE");
  //  log_w("WARNING");
  //  Serial.printf("%02U:%02u:%03u - INFO");
  // start the CAN bus at 250 kbps
  setup_can_driver();
}

void whichCommand(uint8_t Cmd, uint8_t subCmd)
{
  // time
  now_micros = micros();
  // In Millisekunden umrechnen
  total_ms = now_micros / 1000;
  // Zerlegen
  minutes = total_ms / 60000;
  seconds = (total_ms / 1000) % 60;
  ms = total_ms % 1000;

  switch (Cmd)
  {
  case 0x00:
    switch (subCmd)
    {
    case 0x01:
      Serial.printf("%02U:%02u:%03u - System Go\r\n", minutes, seconds, ms);
      break;
    case 0x02:
      Serial.printf("%02U:%02u:%03u - System Halt\r\n", minutes, seconds, ms);
      break;
    case 0x03:
      Serial.printf("%02U:%02u:%03u - Lok Nothalt\r\n", minutes, seconds, ms);
      break;
    case 0x04:
      Serial.printf("%02U:%02u:%03u - Lok Zyklus Stopp(beenden)\r\n", minutes, seconds, ms);
      break;
    case 0x05:
      Serial.printf("%02U:%02u:%03u - Lok Datenprotokoll\r\n", minutes, seconds, ms);
      break;
    case 0x06:
      Serial.printf("%02U:%02u:%03u - Gleisprotokoll frei schalten\r\n", minutes, seconds, ms);
      break;
    case 0x07:
      Serial.printf("%02U:%02u:%03u - Fast Read für mfx\r\n", minutes, seconds, ms);
      break;
    case 0x08:
      Serial.printf("%02U:%02u:%03u - Gleisprotokoll frei schalten\r\n", minutes, seconds, ms);
      break;
    case 0x09:
      Serial.printf("%02U:%02u:%03u - System MFX Neuanmeldezähler setzen\r\n", minutes, seconds, ms);
      break;
    case 0x0A:
      Serial.printf("%02U:%02u:%03u - System Überlast\r\n", minutes, seconds, ms);
      break;
    case 0x0B:
      Serial.printf("%02U:%02u:%03u - System Status\r\n", minutes, seconds, ms);
      break;
    case 0x0C:
      Serial.printf("%02U:%02u:%03u - System Kennung\r\n", minutes, seconds, ms);
      break;
    case 0x30:
      Serial.printf("%02U:%02u:%03u - Mfx Seek\r\n", minutes, seconds, ms);
      break;
    case 0x80:
      Serial.printf("%02U:%02u:%03u - System Reset\r\n", minutes, seconds, ms);
      break;
    }
    break;
  case 0x02:
    Serial.printf("%02U:%02u:%03u - Lok Discovery\r\n", minutes, seconds, ms);
    break;
  case 0x04:
    Serial.printf("%02U:%02u:%03u - MFX Bind\r\n", minutes, seconds, ms);
    break;
  case 0x06:
    Serial.printf("%02U:%02u:%03u - MFX Verify\r\n", minutes, seconds, ms);
    break;
  case 0x08:
    Serial.printf("%02U:%02u:%03u - Lok Geschwindigkeit\r\n", minutes, seconds, ms);
    break;
  case 0x0A:
    Serial.printf("%02U:%02u:%03u - Lok Richtung\r\n", minutes, seconds, ms);
    break;
  case 0x0C:
    Serial.printf("%02U:%02u:%03u - Lok Funktion\r\n", minutes, seconds, ms);
    break;
  case 0x0E:
    Serial.printf("%02U:%02u:%03u - Read Config\r\n", minutes, seconds, ms);
    break;
  case 0x10:
    Serial.printf("%02U:%02u:%03u - Write Config\r\n", minutes, seconds, ms);
    break;
  case 0x16:
    Serial.printf("%02U:%02u:%03u - Zubehör Schalten\r\n", minutes, seconds, ms);
    break;
  case 0x18:
    Serial.printf("%02U:%02u:%03u - Zubehör Konfig\r\n", minutes, seconds, ms);
    break;
  case 0x20:
    Serial.printf("%02U:%02u:%03u - S88 Polling (Feedback)\r\n", minutes, seconds, ms);
    break;
  case 0x22:
    Serial.printf("%02U:%02u:%03u - S88 Event\r\n", minutes, seconds, ms);
    break;
  case 0x24:
    Serial.printf("%02U:%02u:%03u - SX1 Event\r\n", minutes, seconds, ms);
    break;
  case 0x30:
    Serial.printf("%02U:%02u:%03u - Softwarestand Anfrage/Teilnehmer Ping\r\n", minutes, seconds, ms);
    break;
  case 0x32:
    Serial.printf("%02U:%02u:%03u - Updateangebot\r\n", minutes, seconds, ms);
    break;
  case 0x34:
    Serial.printf("%02U:%02u:%03u - Read Config Data\r\n", minutes, seconds, ms);
    break;
  case 0x36:
    Serial.printf("%02U:%02u:%03u - Bootloader CAN gebunden, „Service“\r\n", minutes, seconds, ms);
    break;
  case 0x38:
    Serial.printf("%02U:%02u:%03u - Bootloader Schienen gebunden, „Service“\r\n", minutes, seconds, ms);
    break;
  case 0x3A:
    Serial.printf("%02U:%02u:%03u - Statusdaten Konfiguration\r\n", minutes, seconds, ms);
    break;
  case 0x40:
    Serial.printf("%02U:%02u:%03u - Anfordern Config Data, „Data Query“\r\n", minutes, seconds, ms);
    break;
  case 0x42:
    Serial.printf("%02U:%02u:%03u - Config Data Stream\r\n", minutes, seconds, ms);
    break;
  case 0x44:
    Serial.printf("%02U:%02u:%03u - 60128 (Connect 6021) Data Stream/\n alte Bezeichnung „6021 adapter“\r\n", minutes, seconds, ms);
    break;
  default:
    if (Cmd % 2 == 0)
    {
      Serial.printf("%02U:%02u:%03u - unbekannt: 0x%02X\r\n", minutes, seconds, ms, Cmd);
    }
    else
    {
      Serial.printf("%02U:%02u:%03u - Antwort: 0x%02X\r\n", minutes, seconds, ms, Cmd);
    }
    break;
  }
}

// sendet CAN-Frames vom  CAN (Gleisbox) zum SYS
void proc_fromCAN()
{
  twai_message_t MessageReceived;
  if (twai_receive(&MessageReceived, 3 * portTICK_PERIOD_MS) == ESP_OK)
  {
    // read a packet from CAN
    uint8_t UDPbuffer[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    MessageReceived.identifier &= CAN_EFF_MASK;
    MessageReceived.identifier = htonl(MessageReceived.identifier);
    memcpy(UDPbuffer, &MessageReceived.identifier, 4);
    UDPbuffer[4] = MessageReceived.data_length_code;
    memcpy(&UDPbuffer[5], MessageReceived.data, MessageReceived.data_length_code);
    whichCommand(UDPbuffer[0x01], UDPbuffer[0x09]);
    log_buf_d(UDPbuffer, CAN_FRAME_SIZE);
    Serial.println();
  }
}

void loop()
{
  // die folgenden Routinen werden ständig aufgerufen
  proc_fromCAN();
  delay(10);
}