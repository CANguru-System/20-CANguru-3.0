
/* ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <CANguru-Buch@web.de> wrote this file. As long as you retain this
 * notice you can do whatever you want with this stuff. If we meet some day,
 * and you think this stuff is worth it, you can buy me a beer in return
 * Gustav Wostrack
 * ----------------------------------------------------------------------------
 */

#include <Arduino.h>
#include <espnow.h>
#include "Preferences.h"
#include <CANguruDefs.h>
#include <CAN_const.h>
#include "MOD-LCD.h"
#include <telnet.h>
#include "CANguru.h"
#include <List.hpp>

uint8_t slaveCnt;
uint8_t slaveCurr;
uint8_t nbrSlavesAreReady;
uint8_t wantedscanResults;

struct macType
{
  uint8_t m[macLen];
};

// willkürlich festgelegte MAC-Adresse
const uint8_t masterCustomMac[] = {0x30, 0xAE, 0xA4, 0x89, 0x92, 0x71};

slaveInfoStruct slaveInfo[maxSlaves];
String ssidSLV = "CNgrSLV";

bool SYSseen;
uint8_t cntConfig;

// identifiziert einen Slave anhand seiner UID
uint8_t matchUID(uint8_t *buffer)
{
  uint8_t slaveCnt = get_slaveCnt();
  for (uint8_t s = 0; s < slaveCnt; s++)
  {
    uint32_t uid = UID_BASE + s;
    if (
        (buffer[5] == (uint8_t)(uid >> 24)) &&
        (buffer[6] == (uint8_t)(uid >> 16)) &&
        (buffer[7] == (uint8_t)(uid >> 8)) &&
        (buffer[8] == (uint8_t)uid))
      return s;
  }
  return 0xFF;
}

// ESPNow wird initialisiert
void espInit()
{
  cntConfig = 0;
  slaveCnt = 0;
  slaveCurr = 0;
  initVariant();
  if (esp_now_init() == ESP_OK)
  {
    displayLCD("ESPNow started!");
  }
  else
  {
    displayLCD("ESP_Now INIT FAILED....");
  }
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
}

// gibt die Anzahl der gefundenen Slaves zurück
uint8_t get_slaveCnt()
{
  return slaveCnt;
}

// gibt an, ob SYS gestartet ist
bool get_SYSseen()
{
  return SYSseen;
}

// setzt, dass SYS gestartet ist
void set_SYSseen(bool SYS)
{
  SYSseen = SYS;
}

// liefert die Struktur slaveInfo zurück
slaveInfoStruct get_slaveInfo(uint8_t slave)
{
  return slaveInfo[slave];
}

// gibt eine MAC-Adresse aus
void printMac(uint8_t m[macLen])
{
  char macStr[18] = {0};
  sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", m[0], m[1], m[2], m[3], m[4], m[5]);
  displayLCD(macStr);
}

// vergleicht zwei MAC-Adressen
bool macIsEqual(const uint8_t m0[macLen], const uint8_t m1[macLen])
{
  for (uint8_t ii = 0; ii < macLen; ++ii)
  {
    if (m0[ii] != m1[ii])
    {
      return false;
    }
  }
  return true;
}

void setCntDecoders(uint8_t cnt)
{
  wantedscanResults = cnt;
}

// Wir suchen nach Slaves
// Scannt nach Slaves
void Scan4Slaves()
{
  macType mac;
  const uint8_t shift = 8;
  // Create an immutable list
  List<macType> scanList;
  const uint8_t scanTrial = 10;
  uint8_t scanResults;
  displayLCD("Suche Slaves ...");
  msgStartScanning();
  uint8_t tmpscanResults;
  // für widerspenstige Dekoder machen wir mehrere Durchläufe
  for (uint8_t t = 0; t < scanTrial; t++)
  {
    // lies alle Strahler ein
    scanResults = WiFi.scanNetworks(false, false, false, 5000);
    tmpscanResults = 0;
    // interessant sind die, die mit den richtigen Zeichen beginnen
    // Prüfen ob der Netzname mit `Slave` beginnt
    for (uint8_t i = 0; i < scanResults; i++)
    {
      if (WiFi.SSID(i).indexOf(ssidSLV) == 0)
      {
        // Ja, dann haben wir einen Slave gefunden
        // MAC-Adresse aus der BSSID ses Slaves ermitteln und in der Slave info struktur speichern
        if (macLen == sscanf(WiFi.BSSIDstr(i).c_str(), "%X:%X:%X:%X:%X:%X", &mac.m[0], &mac.m[1], &mac.m[2], &mac.m[3], &mac.m[4], &mac.m[5]))
          scanList.add(mac);
        tmpscanResults++;
      }
    }
    char chs[50];
    sprintf(chs, "Scan# %d - %d slave(s) found of %d!", t, tmpscanResults, wantedscanResults);
    displayLCD(chs);
    if ((tmpscanResults >= wantedscanResults) || (tmpscanResults == 0))
    {
      scanResults = tmpscanResults;
      break;
    }
  }
  if (scanResults == 0)
  {
    displayLCD("Noch kein WiFi Gerät im AP Modus gefunden");
  }
  else
  {
    for (int8_t i = 0; i < scanResults; ++i)
    {
      mac = scanList.get(i);
      bool notAlreadyFound = true;
      for (uint8_t s = 0; s < slaveCnt; s++)
      {
        if (macIsEqual(slaveInfo[s].slave.peer_addr, mac.m) == true)
        {
          notAlreadyFound = false;
          break;
        }
      }
      if (notAlreadyFound == true)
      {
        memcpy(&slaveInfo[slaveCnt].slave.peer_addr, &mac.m, macLen);
        slaveInfo[slaveCnt].slave.channel = WIFI_CHANNEL;
        slaveInfo[slaveCnt].slave.encrypt = 0;
        slaveInfo[slaveCnt].peer = &slaveInfo[slaveCnt].slave;
        // integer äquivalent zu mac adresse ausrechnen; damit werden die mac-adressen sortiert
        slaveInfo[slaveCnt].intValue = 0;
        for (uint8_t j = 0; j < macLen; j++)
          slaveInfo[slaveCnt].intValue = (slaveInfo[slaveCnt].intValue << shift) + mac.m[j];
        slaveInfo[slaveCnt].decoderIsAlive = isAlive;
        slaveCnt++;
      }
    }
  }
  // clean up ram
  WiFi.scanDelete();
}

// setzt die vorgegebene MAC-Adresse des Masters
void initVariant()
{
  WiFi.mode(WIFI_MODE_STA);
  esp_err_t setMacResult = esp_wifi_set_mac((wifi_interface_t)ESP_IF_WIFI_STA, &masterCustomMac[0]);
  if (setMacResult == ESP_OK)
    log_i("Init Variant ok!");
  else
    log_e("Init Variant failed!");
  WiFi.disconnect();
}

void assign(slaveInfoStruct dest, slaveInfoStruct source)
{
  memcpy(&dest.slave.peer_addr, &source.slave.peer_addr, macLen);
  dest.slave.channel = WIFI_CHANNEL;
  dest.slave.encrypt = 0;
  dest.peer = &slaveInfo[slaveCnt].slave;
  // integer äquivalent zu mac adresse ausrechnen; damit werden die mac-adressen sortiert
  dest.intValue = source.intValue;
  slaveInfo[slaveCnt].decoderIsAlive = isAlive;
}

void InsertionSort()
{
  int32_t j = 0;
  slaveInfoStruct slaveInfoKey;

  for (uint8_t i = 1; i < slaveCnt; i++)
  {
    // slaveInfoKey = slaveInfo[i];
    assign(slaveInfoKey, slaveInfo[i]);
    j = i - 1;

    // Move elements of list[0..i-1], that are
    // greater than key, to one position ahead
    // of their current position
    while (j >= 0 && (slaveInfo[j].intValue > slaveInfoKey.intValue))
    {
      //      list[j + 1] = list[j];
      //      slaveInfo[j + 1] = slaveInfo[j];
      assign(slaveInfo[j + 1], slaveInfo[j]);
      j = j - 1;
    }
    // list[j + 1] = key;
    //    slaveInfo[j + 1] = slaveInfoKey;
    assign(slaveInfo[j + 1], slaveInfoKey);
  }
};

// addiert und registriert gefundene Slaves
void addSlaves()
{
  uint8_t Clntbuffer[CAN_FRAME_SIZE]; // buffer to hold incoming packet,
  // die mac-adressen werden sortiert; dadurch
  // werden die angeschlossenen Decoder stets in der gleichen Reihenfolge aufgerufen und
  // weisen sich immer die gleiche UID zu.
  InsertionSort();
  for (uint8_t s = 0; s < slaveCnt; s++)
  {
    memcpy(&Clntbuffer, slaveInfo[s].slave.peer_addr, macLen);
    // device-Nummer übermitteln
    Clntbuffer[macLen] = s;
    sendTheData(s, Clntbuffer, macLen + 1);
    printMac(slaveInfo[s].slave.peer_addr);
    char chs[30];
    sprintf(chs, " -- Added Slave %d\r\n", s + 1);
    displayLCD(chs);
  }
}

// steuert den Registrierungsprozess der Slaves
void registerSlaves()
{
  if (slaveCnt > 0)
  {
    // add slaves
    addSlaves();
    delay(50);
    // alle slaves sind bekannt
    char chs[30];
    int sc = slaveCnt;
    sprintf(chs, "%d slave(s) found!\r\n", sc);
    displayLCD(chs);
    delay(500);
    clearDisplay();
  }
}

// Fehlermeldungen
void printESPNowError(esp_err_t Result)
{
  if (Result == ESP_ERR_ESPNOW_NOT_INIT)
  {
    // How did we get so far!!
    displayLCD("ESPNOW not Init.");
  }
  else if (Result == ESP_ERR_ESPNOW_ARG)
  {
    displayLCD("Invalid Argument");
  }
  else if (Result == ESP_ERR_ESPNOW_INTERNAL)
  {
    displayLCD("Internal Error");
  }
  else if (Result == ESP_ERR_ESPNOW_NO_MEM)
  {
    displayLCD("ESP_ERR_ESPNOW_NO_MEM");
  }
  else if (Result == ESP_ERR_ESPNOW_NOT_FOUND)
  {
    if (slaveCnt > 0)
      displayLCD("Peer not found!");
  }
  else if (Result == ESP_ERR_ESPNOW_IF)
  {
    displayLCD("Interface Error.");
  }
  else
  {
    int res = Result;
    char chs[30];
    sprintf(chs, "\r\nNot sure what happened\t%d", res);
    displayLCD(chs);
  }
}

// speichert den Status eines Decoders/Slaves
void setAlive(uint8_t cnt, aliveStatus a)
{
  slaveInfo[cnt].decoderIsAlive = a;
}

// ruft den Status eines Decoders/Slaves ab
aliveStatus getAlive(uint8_t cnt)
{
  return slaveInfo[cnt].decoderIsAlive;
}

// sendet daten über ESPNow
// der slave wird mit der nummer angesprochen, die sich durch die Reihenfolge beim Erkennen (scannen) ergibt
void sendTheData(uint8_t slave, const uint8_t *data, size_t len)
{
  delay(5);
  if (esp_now_add_peer(slaveInfo[slave].peer) == ESP_OK)
  {
    esp_err_t sendResult = esp_now_send(slaveInfo[slave].slave.peer_addr, data, len);
    if (sendResult != ESP_OK)
    {
      printESPNowError(sendResult);
    }

    esp_now_del_peer(slaveInfo[slave].slave.peer_addr);
  }
  else
    log_i("esp_now_add_peer faild Slave: %d", slave);
}

void setallSlavesAreReadyToZero()
{
  nbrSlavesAreReady = 0;
}

void incSlavesAreReadyToZero()
{
  nbrSlavesAreReady++;
}

uint8_t getallSlavesAreReady()
{
  return nbrSlavesAreReady;
}

// nach dem Versand von Meldungen können hier Fehlermeldungen abgerufen werden
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
}

// empfängt Daten über ESPNow
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len)
{
  uint8_t Clntbuffer[CAN_FRAME_SIZE]; // buffer to hold incoming packet,
  memcpy(Clntbuffer, data, data_len);
  if ((data_len == macLen) || (data_len == macLen + 1))
  {
    // Rückmeldung der slaves, nachdem sie ihre UID festgelegt haben
    // mit nbrSlavesAreReady wird die Anzahl der Rückmeldungen gezählt
    nbrSlavesAreReady++;
    return;
  }
/*  if (data_len == macLen + 1)
  {
    // Rückmeldung der slaves, nachdem sie ihre UID festgelegt haben
    // mit nbrSlavesAreReady wird die Anzahl der Rückmeldungen gezählt
    nbrSlavesAreReady++;
    log_i("macLen+1 %d", nbrSlavesAreReady);
    return;
  }*/
  switch (data[0x01])
  {
  case PING_R:
    sendToServer(Clntbuffer, fromClnt);
    break;
  case CONFIG_Status_R:
    sendToServer(Clntbuffer, fromClnt);
    break;
  case SEND_IP_R:
    sendToServer(Clntbuffer, fromClnt);
    if (!SYSseen)
    {
      cntConfig++;
      if (cntConfig == slaveCnt)
        goSYS();
    }
    break;
  case S88_EVENT_R:
    // Meldungen vom Gleisbesetztmelder
    // nur wenn Win-DigiPet gestartet ist
    if (SYSseen)
    {
      // an SYS
      sendToWDP(Clntbuffer);
    }
    break;
  case sendCurrAmp:
    for (uint8_t i = 0x05; i < 0x05 + 0x08; i++)
    {
      setAmpere(i - 0x05, data[i]);
    }
    break;
  case BlinkAlive_R:
    setAlive(data[0x06], isAlive);
    break;
  default:
    // send received data via Ethernet to GW and evtl to SYS
    sendToServer(Clntbuffer, fromClnt);
    if (SYSseen)
    {
      sendToWDP(Clntbuffer);
    }
    break;
  }
}
