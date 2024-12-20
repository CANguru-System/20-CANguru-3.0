
/* ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <CANguru-Buch@web.de> wrote this file. As long as you retain this
 * notice you can do whatever you want with this stuff. If we meet some day,
 * and you think this stuff is worth it, you can buy me a beer in return
 * Gustav Wostrack
 * ----------------------------------------------------------------------------
 */
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include "Preferences.h"
#include "CANguruDefs.h"
#include "esp32-hal-ledc.h"
#include "esp_system.h"
#include <esp_now.h>
#include <esp_wifi.h>
#include <Ticker.h>
#include <ArduinoOTA.h>
#include <OTA_include.h>
#include "PWM.h"
#include <SPI.h>

// EEPROM-Adressen

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

Preferences preferences;
char key[20];

// config-Daten
// Parameter-Kanäle
enum Kanals
{
  Kanal00,
  Kanal01,
  Kanal02,
  Kanal03,
  Kanal04,
  Kanal05,
  Kanal06,
  endofKanals
};

Kanals CONFIGURATION_Status_Index = Kanal00;

uint8_t decoderadr;
uint8_t uid_device[uid_num];
uint16_t ms_nativeDelay;

// Zeigen an, ob eine entsprechende Anforderung eingegangen ist
bool CONFIG_Status_Request = false;
bool SYS_CMD_Request = false;
bool RESET_MEM_Request = false;
bool START_OTA_Request = false;
bool SEND_IP_Request = false;

// Timer
boolean statusPING;
boolean bDecoderIsAlive;
boolean initialData2send;

#define VERS_HIGH 0x00 // Versionsnummer vor dem Punkt
#define VERS_LOW 0x01  // Versionsnummer nach dem Punkt

/*
Variablen der Signals & Magnetartikel
*/

// 4 Form-Signale
FormSignalClass FormSignal00;
FormSignalClass FormSignal01;
FormSignalClass FormSignal02;
FormSignalClass FormSignal03;
FormSignalClass FormSignal04;
FormSignalClass FormSignal05;
FormSignalClass FormSignal06;
FormSignalClass FormSignal07;
FormSignalClass FormSignal08;
FormSignalClass FormSignal09;
FormSignalClass FormSignal10;
FormSignalClass FormSignal11;
FormSignalClass FormSignal12;
FormSignalClass FormSignal13;
FormSignalClass FormSignal14;
FormSignalClass FormSignal15;
FormSignalClass FormSignals[num_FormSignals] = {FormSignal00, FormSignal01, FormSignal02, FormSignal03,
                                                FormSignal04, FormSignal05, FormSignal06, FormSignal07,
                                                FormSignal08, FormSignal09, FormSignal10, FormSignal11,
                                                FormSignal12, FormSignal13, FormSignal14, FormSignal15};

// 4 LED-Signale
LEDSignalClass LEDSignal00;
LEDSignalClass LEDSignal01;
LEDSignalClass LEDSignal02;
LEDSignalClass LEDSignal03;
LEDSignalClass LEDSignal04;
LEDSignalClass LEDSignal05;
LEDSignalClass LEDSignal06;
LEDSignalClass LEDSignal07;
LEDSignalClass LEDSignals[num_LEDSignals] = {LEDSignal00, LEDSignal01, LEDSignal02, LEDSignal03,
                                             LEDSignal04, LEDSignal05, LEDSignal06, LEDSignal07};

uint8_t LEDsignalDelay;
uint8_t FormsignalDelay;
uint8_t StartAngle;
uint8_t StopAngle;
uint8_t EndAngle;

IPAddress IP;

// Forward declaration
void switchLED(uint16_t acc_num);
void switchForm(uint16_t acc_num);
void LED_report(uint8_t num);
void Form_report(uint8_t num);
void calc_to_address();
void sendConfig();
void generateHash(uint8_t offset);

#include "espnow.h"

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable brownout detector
  Serial.begin(bdrMonitor);
  delay(500);
  log_i("\r\n\r\nF o r m - / L E D - S i g n a l");
  log_i("\n on %s", ARDUINO_BOARD);
  log_i("CPU Frequency = %d Mhz", F_CPU / 1000000);
  //  log_e("ERROR!");
  //  log_d("VERBOSE");
  //  log_w("WARNING");
  //  log_d("INFO");
  // der Decoder strahlt mit seiner Kennung
  // damit kennt die CANguru-Bridge (der Master) seine Decoder findet
  DEVTYPE = DEVTYPE_MAXISIGNAL;
  startAPMode();
  // der Master (CANguru-Bridge) wird registriert
  addMaster();
  // WLAN -Verbindungen können wieder ausgeschaltet werden
  WiFi.disconnect();
  // die preferences-Library wird gestartet
  if (preferences.begin(prefName, false))
  {
    log_d("Preferences %s erfolgreich gestartet\r\n", prefName);
  }
  uint8_t setup_todo;
  if (preferences.isKey("setup_done"))
    setup_todo = preferences.getUChar("setup_done", 0xFF);
  else
    log_d("setup_done nicht gefunden! Bitte zunächst Installationsroutine aufrufen!");
  if (setup_todo != setup_done)
  {
    // wurde das Setup bereits einmal durchgeführt?
    // dann wird dieser Anteil übersprungen
    // 47, weil das EEPROM (hoffentlich) nie ursprünglich diesen Inhalt hatte
    // zunächst alle Vorgänger löschen
    size_t whatsLeft = preferences.freeEntries(); // this method works regardless of the mode in which the namespace is opened.
    log_d("There are: %u entries available in the namespace table.\n", whatsLeft);

    // setzt die Boardnum anfangs auf 1
    decoderadr = 1;
    preferences.putUChar("decoderadr", decoderadr);
    // Verzögerung LED-Umschaltung
    preferences.putUChar("LEDsignaldelay", stdLEDsignaldelay);
    // Verzögerung Form-Signal-Umschaltung
    preferences.putUChar("Formsignaldelay", stdFormsignaldelay);
    // Startwinkel Formsignal
    preferences.putUChar("StartAngle", stdStartAngle);
    // Startwinkel Formsignal
    preferences.putUChar("StopAngle", stdStopAngle);
    // Überschwingwinkel Formsignal
    preferences.putUChar("endAngle", stdendAngle);
    // Status der Formsignale zu Beginn auf rechts setzen
    for (uint8_t form = 0; form < num_FormSignals; form++)
    {
      sprintf(key, "statusForm%d", form); // Känale vorbesetzen
      preferences.putUChar(key, red);
    }
    // Status der LEDignale zu Beginn auf rechts setzen
    for (uint8_t signal = 0; signal < num_LEDSignals; signal++)
    {
      sprintf(key, "statusLED%d", signal); // Känale vorbesetzen
      preferences.putUChar(key, red);
    }
    // ota auf "FALSE" setzen
    preferences.putUChar("ota", startWithoutOTA);
    //
    // setup_done auf "TRUE" setzen
    preferences.putUChar("setup_done", setup_done);
    //
  }
  else
  {
    // Basisadresse des Decoders
    // darüber sind die Ampaln / Andreaskreuze erreichbar
    uint8_t ota = preferences.getUChar("ota", false);
    if (ota == startWithoutOTA)
    {
      // nach dem ersten Mal Einlesen der gespeicherten Werte
      // Adresse
      decoderadr = readValfromPreferences(preferences, "decoderadr", minadr, minadr, maxadr);
    }
    else
    {
      // ota auf "FALSE" setzen
      // ota auf "FALSE" setzen
      preferences.putUChar("ota", startWithoutOTA);
      Connect2WiFiandOTA(preferences);
    }
  }
  // ab hier werden die Anweisungen bei jedem Start durchlaufen
  // IP-Adresse
  char ip[4]; // prepare a buffer for the data
  if (preferences.isKey("ssid"))
    log_d("SSID OK");
  if (preferences.isKey("password"))
    log_d("PASSWORD OK");
  if (preferences.isKey("IP0"))
    log_d("IP-ADDRESS OK");
  if (preferences.isKey("IP0"))
  {
    if (preferences.getBytes("IP0", ip, 4) == 4)
      for (uint8_t i = 0; i < 4; i++)
      {
        IP[i] = ip[i];
        log_d("IP-ADDRESS %d", IP[i]);
      }
  }
  else
  {
    log_d("IP0 nicht gefunden! Bitte zunaechst Installationsroutine aufrufen!");
    while (true)
    {
      // Hier bleibt das Programm stehen
    }
  }
  // ab hier werden die Anweisungen bei jedem Start durchlaufen
  // Flags
  got1CANmsg = false;
  SYS_CMD_Request = false;
  statusPING = false;
  RESET_MEM_Request = false;
  START_OTA_Request = false;
  SEND_IP_Request = false;
  /////////////
  initPWM_Form();
  FormsignalDelay = readValfromPreferences(preferences, "FormsignalDelay", minFormsignaldelay, minFormsignaldelay, maxFormsignaldelay);
  StartAngle = readValfromPreferences(preferences, "StartAngle", stdStartAngle, stdStartAngle, stdStopAngle);
  StopAngle = readValfromPreferences(preferences, "StopAngle", stdStopAngle, stdStartAngle, stdStopAngle);
  EndAngle = readValfromPreferences(preferences, "EndAngle", stdendAngle, minendAngle, maxendAngle);
  for (uint8_t form = 0; form < num_FormSignals; form++)
  {
    // Status der Magnetartikel versenden an die Servos
    FormSignals[form].Attach(form);
    FormSignals[form].SetStartAngle(StartAngle);
    FormSignals[form].SetStopAngle(StopAngle);
    FormSignals[form].SetEndAngle(EndAngle);
    sprintf(key, "statusForm%d", form); // Känale vorbesetzen
    colorLED status = (colorLED)preferences.getUChar(key);
    FormSignals[form].SetLightDest(status);
    // Signals mit den PINs verbinden, initialisieren & Artikel setzen wie gespeichert
    FormSignals[form].SetDelay(FormsignalDelay);
    FormSignals[form].SetcolorLED();
  }
  /////////////
  initPWM_LED();
  LEDsignalDelay = readValfromPreferences(preferences, "LEDsignalDelay", stdLEDsignaldelay, minLEDsignaldelay, maxLEDsignaldelay);
  for (uint8_t signal = 0; signal < num_LEDSignals; signal++)
  {
    // Status der Magnetartikel versenden an die Servos
    LEDSignals[signal].Attach(signal);
    // Status der Magnetartikel einlesen in lokale arrays
    sprintf(key, "statusLED%d", signal); // Känale vorbesetzen
    colorLED status = (colorLED)preferences.getUChar(key);
    LEDSignals[signal].SetLightDest(status);
    // Signals mit den PINs verbinden, initialisieren & Artikel setzen wie gespeichert
    LEDSignals[signal].SetDelay(LEDsignalDelay);
    LEDSignals[signal].SetcolorLED();
  }
  // berechnet die _to_address aus der Adresse und der Protokollkonstante
  calc_to_address();
  // Vorbereiten der Blink-LED
  stillAliveBlinkSetup(LED_BUILTIN);
}

// Zu Beginn des Programmablaufs werden die aktuellen Statusmeldungen an WDP geschickt
void sendTheInitialData()
{
  initialData2send = false;
  for (uint8_t signal = 0; signal < num_LEDSignals; signal++)
  {
    // Status der LED-Signale melden
    LED_report(signal);
  }
  for (uint8_t form = 0; form < num_FormSignals; form++)
  {
    // Status der Form-Signale melden
    Form_report(form);
  }
}

/*
CAN Grundformat
Das CAN Protokoll schreibt vor, dass Meldungen mit einer 29 Bit Meldungskennung,
4 Bit Meldungslänge sowie bis zu 8 Datenbyte bestehen.
Die Meldungskennung wird aufgeteilt in die Unterbereiche Priorit�t (Prio),
Kommando (Command), Response und Hash.
Die Kommunikation basiert auf folgendem Datenformat:

Meldungskennung
Prio	2+2 Bit Message Prio			28 .. 25
Command	8 Bit	Kommando Kennzeichnung	24 .. 17
Resp.	1 Bit	CMD / Resp.				16
Hash	16 Bit	Kollisionsaufl�sung		15 .. 0
DLC
DLC		4 Bit	Anz. Datenbytes
Byte 0	D-Byte 0	8 Bit Daten
Byte 1	D-Byte 1	8 Bit Daten
Byte 2	D-Byte 2	8 Bit Daten
Byte 3	D-Byte 3	8 Bit Daten
Byte 4	D-Byte 4	8 Bit Daten
Byte 5	D-Byte 5	8 Bit Daten
Byte 6	D-Byte 6	8 Bit Daten
Byte 7	D-Byte 7	8 Bit Daten
*/

// Die Adressen der einzelnen Servos werden berechnet,
// dabei wird die Decoderadresse einbezogen, so dass
// das erste Servo die Adresse
// Protokollkonstante (0x3000) + (Decoderadresse-1) * Anzahl Servos pro Decoder
// erhält. Das zweite Servo hat dann die Adresse Servo1 plus 1; die
// folgende analog
// das Kürzel to an verschiedenen Stellen steht für den englischen Begriff
// für Weiche, nämlich turnout
void calc_to_address()
{
  uint16_t baseAddress = (decoderadr - 1) * num_accessory; // num_accessory immer 4
  // berechnet die _to_address aus der Adresse und der Protokollkonstante
  // Formsignale
  for (uint8_t form = 0; form < num_FormSignals; form++)
  {
    uint16_t to_address = PROT + baseAddress + form;
    // _to_addresss einlesen in lokales array
    FormSignals[form].Set_to_address(to_address);
    Form_report(form);
  }
  baseAddress += num_FormSignals;
  // LED-Signale
  for (uint8_t signal = 0; signal < num_LEDSignals; signal++)
  {
    uint16_t to_address = PROT + baseAddress + signal;
    // _to_addresss einlesen in lokales array
    LEDSignals[signal].Set_to_address(to_address);
    LED_report(signal);
  }
}

// receiveKanalData dient der Parameterübertragung zwischen Decoder und CANguru-Server
// es erhält die evtuelle auf dem Server geänderten Werte zurück
void receiveKanalData()
{
  SYS_CMD_Request = false;
  uint8_t oldval;
  switch (opFrame[data5])
  {
  // Kanalnummer #1 - Decoderadresse
  case Kanal01:
  {
    oldval = decoderadr;
    decoderadr = (opFrame[data6] << 8) + opFrame[data7];
    if (testMinMax(oldval, decoderadr, minadr, maxadr))
    {
      // speichert die neue Adresse
      preferences.putUChar("decoderadr", decoderadr);
      // neue Adressen
      calc_to_address();
    }
    else
    {
      decoderadr = oldval;
    }
  }
  break;
  // Kanalnummer #2 - Signalverzögerung LED-Umschaltung
  case Kanal02:
  {
    oldval = LEDsignalDelay;
    LEDsignalDelay = (opFrame[data6] << 8) + opFrame[data7];
    if (testMinMax(oldval, LEDsignalDelay, minLEDsignaldelay, maxLEDsignaldelay))
    {
      preferences.putUChar("LEDsignalDelay", LEDsignalDelay);
      for (int signal = 0; signal < num_LEDSignals; signal++)
      {
        // neue Verzögerung
        LEDSignals[signal].SetDelay(LEDsignalDelay);
      }
    }
    else
    {
      LEDsignalDelay = oldval;
    }
  }
  break;
  // Kanalnummer #3 - Signalverzögerung Form-Signal-Umschaltung
  case Kanal03:
  {
    oldval = FormsignalDelay;
    FormsignalDelay = (opFrame[data6] << 8) + opFrame[data7];
    if (testMinMax(oldval, FormsignalDelay, minFormsignaldelay, maxFormsignaldelay))
    {
      preferences.putUChar("FormsignalDelay", FormsignalDelay);
      for (int form = 0; form < num_FormSignals; form++)
      {
        // neue Verzögerung
        FormSignals[form].SetDelay(FormsignalDelay);
      }
    }
    else
    {
      FormsignalDelay = oldval;
    }
  }
  break;
  // Kanalnummer #4 - Startwinkel Form-Signal
  case Kanal04:
  {
    oldval = StartAngle;
    StartAngle = (opFrame[data6] << 8) + opFrame[data7];
    if (testMinMax(oldval, StartAngle, stdStartAngle, stdStopAngle))
    {
      preferences.putUChar("StartAngle", StartAngle);
      for (int form = 0; form < num_FormSignals; form++)
      {
        // neue Verzögerung
        FormSignals[form].SetStartAngle(StartAngle);
      }
    }
    else
    {
      StartAngle = oldval;
    }
  }
  break;
  // Kanalnummer #5 - Stoppwinkel Form-Signal
  case Kanal05:
  {
    oldval = StopAngle;
    StopAngle = (opFrame[data6] << 8) + opFrame[data7];
    if (testMinMax(oldval, StopAngle, stdStartAngle, stdStopAngle))
    {
      preferences.putUChar("StopAngle", StopAngle);
      for (int form = 0; form < num_FormSignals; form++)
      {
        // neue Verzögerung
        FormSignals[form].SetStopAngle(StopAngle);
      }
    }
    else
    {
      StopAngle = oldval;
    }
  }
  break;
  // Kanalnummer #6 - Überschwingwinkel Form-Signal
  case Kanal06:
  {
    oldval = EndAngle;
    EndAngle = (opFrame[data6] << 8) + opFrame[data7];
    if (testMinMax(oldval, EndAngle, minendAngle, maxendAngle))
    {
      preferences.putUChar("EndAngle", EndAngle);
      for (int form = 0; form < num_FormSignals; form++)
      {
        // neuer Überschwingwinkel
        FormSignals[form].SetEndAngle(EndAngle);
      }
    }
    else
    {
      EndAngle = oldval;
    }
  }
  break;
  }
  //
  opFrame[11] = 0x01;
  opFrame[4] = 0x07;
  sendCanFrame();
}

// Routine meldet an die CANguru-Bridge, wenn eine Statusänderung
// einer Ampel eingetreten ist
void Form_report(uint8_t num)
{
  opFrame[CANcmd] = SWITCH_ACC;
  opFrame[Framelng] = 0x05;
  // Weichenadresse
  opFrame[data0] = 0x00;
  opFrame[data1] = 0x00;
  opFrame[data2] = (uint8_t)(FormSignals[num].Get_to_address() >> 8);
  opFrame[data3] = (uint8_t)FormSignals[num].Get_to_address();
  // Meldung der Lage
  opFrame[data4] = FormSignals[num].GetLightCurr();
  sendCanFrame();
  delay(wait_time); // Delay added just so we can have time to open up
}

// Routine meldet an die CANguru-Bridge, wenn eine Statusänderung
// einer Ampel eingetreten ist
void LED_report(uint8_t num)
{
  opFrame[CANcmd] = SWITCH_ACC;
  opFrame[Framelng] = 0x05;
  // Weichenadresse
  opFrame[data0] = 0x00;
  opFrame[data1] = 0x00;
  opFrame[data2] = (uint8_t)(LEDSignals[num].Get_to_address() >> 8);
  opFrame[data3] = (uint8_t)LEDSignals[num].Get_to_address();
  // Meldung der Lage
  opFrame[data4] = LEDSignals[num].GetLightCurr();
  sendCanFrame();
  delay(wait_time); // Delay added just so we can have time to open up
}

void switchForm(uint16_t acc_num)
{
  colorLED set_light = FormSignals[acc_num].GetLightDest();
  FormSignals[acc_num].SetcolorLED();
  Form_report(acc_num);
  sprintf(key, "statusForm%d", acc_num);
  preferences.putUChar(key, (uint8_t)set_light);
}

// Diese Routine leitet den Positionswechsel einer Weiche/Signal ein.
void switchLED(uint16_t acc_num)
{
  colorLED set_light = LEDSignals[acc_num].GetLightDest();
  LEDSignals[acc_num].SetLightCurr(set_light);
  LEDSignals[acc_num].SetcolorLED();
  LED_report(acc_num);
  sprintf(key, "statusLED%d", acc_num);
  preferences.putUChar(key, (uint8_t)set_light);
}

// auf Anforderung des CANguru-Servers sendet der Decoder
// mit dieser Prozedur sendConfig seine Parameterwerte
void sendConfig()
{
  const uint8_t Kanalwidth = 8;
  const uint8_t numberofKanals = endofKanals - 1;

  const uint8_t NumLinesKanal00 = 5 * Kanalwidth;
  uint8_t arrKanal00[NumLinesKanal00] = {
      /*1*/ Kanal00, numberofKanals, (uint8_t)0, (uint8_t)0, (uint8_t)0, (uint8_t)0, (uint8_t)0, decoderadr,
      /*2.1*/ (uint8_t)highbyte2char(hex2dec(uid_device[0])), (uint8_t)lowbyte2char(hex2dec(uid_device[0])),
      /*2.2*/ (uint8_t)highbyte2char(hex2dec(uid_device[1])), (uint8_t)lowbyte2char(hex2dec(uid_device[1])),
      /*2.3*/ (uint8_t)highbyte2char(hex2dec(uid_device[2])), (uint8_t)lowbyte2char(hex2dec(uid_device[2])),
      /*2.4*/ (uint8_t)highbyte2char(hex2dec(uid_device[3])), (uint8_t)lowbyte2char(hex2dec(uid_device[3])),
      /*3*/ 'C', 'A', 'N', 'g', 'u', 'r', 'u', ' ',
      /*4*/ 'M', 'a', 'x', 'i', '-', 'S', 'i', 'g',
      /*5*/ 'n', 'a', 'l', 0, 0, 0, 0, 0};
  const uint8_t NumLinesKanal01 = 4 * Kanalwidth;
  uint8_t arrKanal01[NumLinesKanal01] = {
      // #2 - WORD immer Big Endian, wie Uhrzeit
      /*1*/ Kanal01, 2, 0, 1, 0, maxadr, 0, decoderadr,
      /*2*/ 'M', 'o', 'd', 'u', 'l', 'a', 'd', 'r',
      /*3*/ 'e', 's', 's', 'e', 0, '1', 0, (maxadr / 100) + '0',
      /*4*/ (maxadr - (uint8_t)(maxadr / 100) * 100) / 10 + '0', (maxadr - (uint8_t)(maxadr / 10) * 10) + '0', 0, 'A', 'd', 'r', 0, 0};
  const uint8_t NumLinesKanal02 = 4 * Kanalwidth;
  uint8_t arrKanal02[NumLinesKanal02] = {
      /*1*/ Kanal02, 2, 0, minLEDsignaldelay, 0, maxLEDsignaldelay, 0, LEDsignalDelay,
      /*2*/ 'D', 'e', 'l', 'a', 'y', ' ', 'L', 'e',
      /*3*/ 'd', 0, minLEDsignaldelay + '0', 0, (maxLEDsignaldelay / 100) + '0', (maxLEDsignaldelay - (uint8_t)(maxLEDsignaldelay / 100) * 100) / 10 + '0', (maxLEDsignaldelay - (uint8_t)(maxLEDsignaldelay / 10) * 10) + '0', 0,
      /*4*/ 'm', 's', 0, 0, 0, 0, 0, 0};
  const uint8_t NumLinesKanal03 = 4 * Kanalwidth;
  uint8_t arrKanal03[NumLinesKanal03] = {
      /*1*/ Kanal03, 2, 0, minFormsignaldelay, 0, maxFormsignaldelay, 0, FormsignalDelay,
      /*2*/ 'D', 'e', 'l', 'a', 'y', ' ', 'F', 'o',
      /*3*/ 'r', 'm', 0, minFormsignaldelay + '0', 0, (maxFormsignaldelay / 100) + '0', (maxFormsignaldelay - (uint8_t)(maxFormsignaldelay / 100) * 100) / 10 + '0', (maxFormsignaldelay - (uint8_t)(maxFormsignaldelay / 10) * 10) + '0',
      /*4*/ 0, 'm', 's', 0, 0, 0, 0, 0};
  const uint8_t NumLinesKanal04 = 4 * Kanalwidth;
  uint8_t arrKanal04[NumLinesKanal04] = {
      /*1*/ Kanal04, 2, 0, stdStartAngle, 0, stdStopAngle, 0, StartAngle,
      /*2*/ 'S', 't', 'a', 'r', 't', 'w', 'i', 'n',
      /*3*/ 'k', 'e', 'l', 0, stdStartAngle + '0', 0, (stdStopAngle / 100) + '0', (stdStopAngle - (uint8_t)(stdStopAngle / 100) * 100) / 10 + '0',
      /*4*/ (stdStopAngle - (uint8_t)(stdStopAngle / 10) * 10) + '0', 0, 'g', 'r', 'a', 'd', 0, 0};
  const uint8_t NumLinesKanal05 = 4 * Kanalwidth;
  uint8_t arrKanal05[NumLinesKanal05] = {
      /*1*/ Kanal05, 2, 0, stdStartAngle, 0, stdStopAngle, 0, StopAngle,
      /*2*/ 'S', 't', 'o', 'p', 'p', 'w', 'i', 'n',
      /*3*/ 'k', 'e', 'l', 0, stdStartAngle + '0', 0, (stdStopAngle / 100) + '0', (stdStopAngle - (uint8_t)(stdStopAngle / 100) * 100) / 10 + '0',
      /*4*/ (stdStopAngle - (uint8_t)(stdStopAngle / 10) * 10) + '0', 0, 'g', 'r', 'a', 'd', 0, 0};
  const uint8_t NumLinesKanal06 = 5 * Kanalwidth;
  uint8_t arrKanal06[NumLinesKanal06] = {
      /*1*/ Kanal06, 2, 0, minendAngle, 0, maxendAngle, 0, readValfromPreferences(preferences, "EndAngle", stdendAngle, minendAngle, maxendAngle),
      /*2*/ 0xDC, 'b', 'e', 'r', 's', 'c', 'h', 'w',
      /*3*/ 'i', 'n', 'g', 'w', 'i', 'n', 'k', 'e',
      /*4*/ 'l', 0, minendAngle + '0', 0, maxendAngle / 10 + '0', (maxendAngle - (uint8_t)(maxendAngle / 10) * 10) + '0', 0, 'g',
      /*5*/ 'r', 'a', 'd', 0, 0, 0, 0, 0};
  uint8_t NumKanalLines[numberofKanals + 1] = {
      NumLinesKanal00, NumLinesKanal01, NumLinesKanal02, NumLinesKanal03, NumLinesKanal04, NumLinesKanal05, NumLinesKanal06};

  uint8_t paket = 0;
  uint8_t outzeichen = 0;
  CONFIG_Status_Request = false;
  for (uint8_t inzeichen = 0; inzeichen < NumKanalLines[CONFIGURATION_Status_Index]; inzeichen++)
  {
    opFrame[1] = CONFIG_Status + 1;
    switch (CONFIGURATION_Status_Index)
    {
    case Kanal00:
    {
      opFrame[outzeichen + 5] = arrKanal00[inzeichen];
    }
    break;
    case Kanal01:
    {
      opFrame[outzeichen + 5] = arrKanal01[inzeichen];
    }
    break;
    case Kanal02:
    {
      opFrame[outzeichen + 5] = arrKanal02[inzeichen];
    }
    break;
    case Kanal03:
    {
      opFrame[outzeichen + 5] = arrKanal03[inzeichen];
    }
    break;
    case Kanal04:
    {
      opFrame[outzeichen + 5] = arrKanal04[inzeichen];
    }
    break;
    case Kanal05:
    {
      opFrame[outzeichen + 5] = arrKanal05[inzeichen];
    }
    break;
    case Kanal06:
    {
      opFrame[outzeichen + 5] = arrKanal06[inzeichen];
    }
    break;
    case endofKanals:
    {
      // der Vollständigkeit geschuldet
    }
    break;
    }
    outzeichen++;
    if (outzeichen == 8)
    {
      opFrame[4] = 8;
      outzeichen = 0;
      paket++;
      opFrame[2] = 0x00;
      opFrame[3] = paket;
      sendTheData();
      delay(wait_time_small);
    }
  }
  //
  memset(opFrame, 0, sizeof(opFrame));
  opFrame[1] = CONFIG_Status + 1;
  opFrame[2] = hasharr[0];
  opFrame[3] = hasharr[1];
  opFrame[4] = 0x06;
  for (uint8_t i = 0; i < 4; i++)
  {
    opFrame[i + 5] = uid_device[i];
  }
  opFrame[9] = CONFIGURATION_Status_Index;
  opFrame[10] = paket;
  sendTheData();
  delay(wait_time_small);
}

// In dieser Schleife verbringt der Decoder die meiste Zeit
void loop()
{
  static uint8_t cross = 0;
  static uint8_t signal = 0;
  // die Formsignale werden permant abgefragt, ob es ein Delta zwischen
  // tatsächlicher und gewünschter Stellung gibt und ggf. korrigiert
  FormSignals[cross].Update();
  cross++;
  if (cross >= num_FormSignals)
    cross = 0;
  // die Ampeln werden permant abgefragt, ob es ein Delta zwischen
  // tatsächlichem und gewünschtem Signallicht gibt und ggf. korrigiert
  LEDSignals[signal].Update();
  signal++;
  if (signal >= num_LEDSignals)
    signal = 0;
  // die boolsche Variable got1CANmsg zeigt an, ob vom Master
  // eine Nachricht gekommen ist; der Zustand der Flags
  // entscheidet dann, welche Routine anschließend aufgerufen wird
  if (got1CANmsg)
  {
    got1CANmsg = false;
    // auf PING Antworten
    if (statusPING)
    {
      sendPING();
    }
    if (bDecoderIsAlive)
    {
      sendDecoderIsAlive();
    }
    // Parameterwerte vom CANguru-Server erhalten
    if (SYS_CMD_Request)
    {
      receiveKanalData();
    }
    // die eigene IP-Adresse wird über die Bridge an den Server zurück geliefert
    if (SEND_IP_Request)
    {
      sendIP();
    }
    // Parameterwerte an den CANguru-Server liefern
    if (CONFIG_Status_Request)
    {
      sendConfig();
    }
    // beim ersten Durchlauf werden Initialdaten an die
    // CANguru-Bridge gesendet
    if (initialData2send)
    {
      sendTheInitialData();
    }
  }
}
