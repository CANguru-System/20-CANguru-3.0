
/* ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <CANguru-Buch@web.de> wrote this file. As long as you retain this
 * notice you can do whatever you want with this stuff. If we meet some day,
 * and you think this stuff is worth it, you can buy me a beer in return
 * Gustav Wostrack
 * ----------------------------------------------------------------------------
 */

#ifndef ON_DATA_RECEIVE_H
#define ON_DATA_RECEIVE_H

#include <Arduino.h>

// alle Meldungen von der CANguru-Bridge kommen hier rein
// und werden hier ausgewertet und evtl. weiter geleitet
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len)
{
  memcpy(opFrame, data, data_len);
  if (data_len == macLen + 1)
  {
    // nur beim handshaking; es werden die macaddress
    // und die devicenummer übermittelt und die macaddress zurückgeschickt
    esp_err_t sendResult = esp_now_send(master.peer_addr, opFrame, macLen);
    if (sendResult != ESP_OK)
    {
      printESPNowError(sendResult);
    }
    generateHash(opFrame[macLen]);
    return;
  }
  switch (opFrame[0x01])
  {
    // bewirkt das Aufblitzen der LED
  case BlinkAlive:
    if (secs < 10)
      secs = 10;
    bDecoderIsAlive = true;
    got1CANmsg = true;
    break;
  case PING:
  {
    statusPING = true;
    got1CANmsg = true;
    // alles Weitere wird in loop erledigt
  }
  break;
  // IP-Adresse rückmelden
  case SEND_IP:
    SEND_IP_Request = true;
    got1CANmsg = true;
    // alles Weitere wird in loop erledigt
    break;
  // config
  case CONFIG_Status:
  {
    CONFIG_Status_Request = true;
    CONFIGURATION_Status_Index = (Kanals)opFrame[data4];
    if (CONFIGURATION_Status_Index > 0 && secs < 100)
      secs = 100;
    got1CANmsg = true;
  }
  break;
  case SYS_CMD:
  {
    switch (opFrame[data4])
    {
    case SYS_STOPP:
      for (boosters booster = booster_0; booster < endbooster; booster = boosters(booster + 1))
        turnPowerOff(booster);
      break;
    case SYS_GO:
      for (boosters booster = booster_0; booster < endbooster; booster = boosters(booster + 1))
        turnPowerOn(booster);
      break;
    case SYS_STAT:
      SYS_CMD_Request = true;
      // alles Weitere wird in loop erledigt
      got1CANmsg = true;
      break;
    case RESET_MEM:
    // setup_done auf "TRUE" setzen
      preferences.putUChar("setup_done", setup_NOT_done);
      ESP.restart();
      break;
    case START_OTA:
    // Modul wird neu gestartet und wartet anschließend auf neue Software (OTA)
      preferences.putUChar("ota", startWithOTA);
      ESP.restart();
      break;
    }
  }
  break;
  //
  case CALL4CONNECT + 1:
  {
    bridgeIsConnected2Server = true;
  }
  break;
  case restartBridge:
    ESP.restart();
    break;
  }
}

#endif