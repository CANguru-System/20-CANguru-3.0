
/* ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <CANguru-Buch@web.de> wrote this file. As long as you retain this
 * notice you can do whatever you want with this stuff. If we meet some day,
 * and you think this stuff is worth it, you can buy me a beer in return
 * Gustav Wostrack
 * ----------------------------------------------------------------------------
 */

#include <WiFi.h>
#include <WiFiUdp.h>
#include "Preferences.h"
#include "Stepper.h"
#include <esp_now.h>
#include <esp_wifi.h>
#include <Ticker.h>
#include <ArduinoOTA.h>
#include <OTA_include.h>
#include "CANguruDefs.h"
#include "OWN_LED.h"

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

Preferences preferences;

// config-Daten
// Parameter-Kanäle
enum Kanals
{
  Kanal00,
  Kanal01,
  Kanal02,
  Kanal03,
  Kanal04,
  endofKanals
};

Kanals CONFIGURATION_Status_Index = Kanal00;

uint8_t uid_device[uid_num];
uint16_t ms_nativeDelay;

// Zeigen an, ob eine entsprechende Anforderung eingegangen ist
bool CONFIG_Status_Request = false;
bool SYS_CMD_Request = false;
bool SEND_IP_Request = false;
bool bBlinkAlive;

// Timer
boolean statusPING;
boolean initialData2send;
boolean bDecoderIsAlive;

#define VERS_HIGH 0x00 // Versionsnummer vor dem Punkt
#define VERS_LOW 0x01  // Versionsnummer nach dem Punkt

/*
Variablen der steppers & Magnetartikel
*/
StepperwButton button(btn_Step_pin);

uint8_t decoderadr;
uint8_t stepperDelay;
uint16_t startPos;
uint16_t endPos;
position DownORUpward;
stepDirections stepDirection;

const uint16_t startPos_min = 0;
const uint16_t startPos_std = 0;
const uint16_t startPos_max = 500;

const uint16_t endPos_min = 100;
const uint16_t endPos_std = 600;
const uint16_t endPos_max = 800;

IPAddress IP;

// Forward declaration
void switchAcc();
void acc_report();
void sendConfig();
void generateHash(uint8_t offset);

#include "espnow.h"

void IRAM_ATTR checkButton()
{
  // include all buttons here to be checked
  // stops the stepper
  setContinue(false);
}

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable brownout detector
  Serial.begin(bdrMonitor);
  uint8_t nativeMACAddress[macLen];
  WiFi.macAddress(nativeMACAddress);
  ms_nativeDelay = nativeMACAddress[macLen - 1];
  // a unique delay is necessary to avoid a jam on the way to the router
  delay(350 + ms_nativeDelay);
  log_d("delay: %d", ms_nativeDelay);
  log_i("\r\n\r\nCANguru - Stepper - Formsignal");
  log_i("\n on %s", ARDUINO_BOARD);
  log_i("CPU Frequency = %d Mhz", F_CPU / 1000000);
  //  log_e("ERROR!");
  //  log_d("VERBOSE");
  //  log_w("WARNING");
  //  log_d("INFO");
  // der Decoder strahlt mit seiner Kennung
  // damit kennt die CANguru-Bridge (der Master) seine Decoder findet
  DEVTYPE = DEVTYPE_STEPPER;
  //  LED_begin(GPIO_NUM_8);
  // der Decoder strahlt mit seiner Kennung
  // damit kennt die CANguru-Bridge (der Master) seine Decoder findet
  startAPMode();
  // der Master (CANguru-Bridge) wird registriert
  addMaster();

  // die preferences-Library wird gestartet

  if (preferences.begin(prefName, false))
  {
    log_d("Preferences erfolgreich gestartet");
  }

  uint8_t setup_todo = preferences.getUChar("setup_done", 0xFF);
  if (setup_todo != setup_done)
  {
    // alles fürs erste Mal
    //
    // wurde das Setup bereits einmal durchgeführt?
    // dann wird dieser Anteil übersprungen
    // 47, weil das preferences (hoffentlich) nie ursprünglich diesen Inhalt hatte

    // setzt die Boardnum anfangs auf 1
    decoderadr = 1;
    preferences.putUChar("decoderadr", decoderadr);
    // Anfangswerte
    // Endposition des Steppers
    startPos = startPos_std;
    preferences.putUShort("startPos", startPos);

    // Startposition des Steppers
    endPos = endPos_std;
    preferences.putUShort("endPos", endPos);
    //
    // Verzögerung
    stepperDelay = stdstepperdelay;
    preferences.putUChar("SrvDel", stepperDelay);
    //
    // Status der Magnetartikel zu Beginn auf down (= Signal steht auf STOPP) setzen
    DownORUpward = Down;
    preferences.putUChar("acc_state", DownORUpward);

    // Ausrichtung des Stepper Motors
    stepDirection = A_dir;
    preferences.putUChar("s_d", stepDirection);

    // ota auf "FALSE" setzen
    preferences.putUChar("ota", startWithoutOTA);
    //
    // setup_done auf "TRUE" setzen
    preferences.putUChar("setup_done", setup_done);
    //
  }
  else
  {
    uint8_t ota = preferences.getUChar("ota", false);
    if (ota == startWithoutOTA)
    {
      // nach dem ersten Mal Einlesen der gespeicherten Werte
      // Adresse
      decoderadr = readValfromPreferences(preferences, "decoderadr", minadr, minadr, maxadr);
      // Verzögerung
      stepperDelay = readValfromPreferences(preferences, "SrvDel", stdstepperdelay, minstepperdelay, maxstepperdelay);
      // Startposition
      startPos = readValfromPreferences16(preferences, "startPos", startPos_std, startPos_min, startPos_max);
      // Endposition
      endPos = readValfromPreferences16(preferences, "endPos", endPos_std, endPos_min, endPos_max);
      // Status der Magnetartikel versenden an die steppers
      DownORUpward = (position)readValfromPreferences(preferences, "acc_state", Down, Down, Upward);
      // Ausrichtung des Stepper Motors
      stepDirection = (stepDirections)readValfromPreferences(preferences, "s_d", A_dir, A_dir, B_dir);
    }
    else
    {
      // ota auf "FALSE" setzen
      preferences.putUChar("ota", startWithoutOTA);
      //
      Connect2WiFiandOTA(preferences);
    }
  }
  // ab hier werden die Anweisungen bei jedem Start durchlaufen
  // IP-Adresse
  char ip[4]; // prepare a buffer for the data
  preferences.getBytes("IP0", ip, 4);
  for (uint8_t i = 0; i < 4; i++)
  {
    IP[i] = ip[i];
  }

  // Flags
  got1CANmsg = false;
  SYS_CMD_Request = false;
  SEND_IP_Request = false;
  statusPING = false;
  initialData2send = false;
  bBlinkAlive = true;
  bDecoderIsAlive = true;
  // Variablen werden gemäß der eingelesenen Werte gesetzt
  // evtl. werden auch die steppers verändert
  // steppers mit den PINs verbinden, initialisieren & Artikel setzen wie gespeichert

  // setup interrupt routine
  attachInterrupt(digitalPinToInterrupt(btn_Step_pin), checkButton, CHANGE);
  button.Set_to_address(decoderadr);
  button.SetDelay(stepperDelay);
  button.Set_endpos(endPos);
  button.SetPosCurr(DownORUpward);
  button.Attach(stepDirection);
  button.SetPosition();
  // Vorbereiten der Blink-LED
  stillAliveBlinkSetup(GPIO_NUM_8);
}

// Zu Beginn des Programmablaufs werden die aktuellen Statusmeldungen an WDP geschickt
void sendTheInitialData()
{
  acc_report();
}

// receiveKanalData dient der Parameterübertragung zwischen Decoder und CANguru-Server
// es erhält die evtuelle auf dem Server geänderten Werte zurück
void receiveKanalData()
{
  SYS_CMD_Request = false;
  uint16_t oldval;
  switch (opFrame[data5])
  {
  // Kanalnummer #1 - Decoderadresse
  case Kanal01:
  {
    oldval = decoderadr;
    decoderadr = (opFrame[data6] << 8) + opFrame[data7];
    if (testMinMax(oldval, decoderadr, minadr, maxadr) && preferences.getUChar("receiveTheData", true))
    {
      // speichert die neue Adresse
      preferences.putUChar("decoderadr", decoderadr);
      button.Set_to_address(decoderadr);
      //
    }
    else
    {
      decoderadr = oldval;
    }
  }
  break;
  // Kanalnummer #2 - stepperverzögerung
  case Kanal02:
  {
    oldval = stepperDelay;
    stepperDelay = (opFrame[data6] << 8) + opFrame[data7];
    if (testMinMax(oldval, stepperDelay, minstepperdelay, maxstepperdelay) && preferences.getUChar("receiveTheData", true))
    {
      preferences.putUChar("SrvDel", stepperDelay);
      button.SetDelay(stepperDelay);
      //
    }
    else
    {
      stepperDelay = oldval;
    }
  }
  break;
  // Kanalnummer #3 - Anfangsposition
  case Kanal03:
  {
    if (button.GetPosDest() == Down)
    // only in DOWN-Position possible
    {
      oldval = startPos;
      startPos = (opFrame[data6] << 8) + opFrame[data7];
      if (testMinMax(oldval, startPos, startPos_min, startPos_max) && preferences.getUChar("receiveTheData", true))
      {
        preferences.putUShort("startPos", startPos);
        button.Set_startpos(startPos);
        if (oldval != startPos)
          button.Move_newstartpos(startPos - oldval);
        //
      }
      else
      {
        startPos = oldval;
      }
    }
  }
  break;
  // Kanalnummer #4 - Endposition
  case Kanal04:
  {

    oldval = endPos;
    endPos = (opFrame[data6] << 8) + opFrame[data7];
    if (testMinMax(oldval, endPos, endPos_min, endPos_max) && preferences.getUChar("receiveTheData", true))
    {
      preferences.putUShort("endPos", endPos);
      button.Set_endpos(endPos);
      //
    }
    else
    {
      endPos = oldval;
    }
  }
  break;
  }
  //
  opFrame[data6] = 0x01;
  opFrame[Framelng] = 0x07;
  sendCanFrame();
}

// Routine meldet an die CANguru-Bridge, wenn eine Statusänderung
// einer Weiche/Signal eingetreten ist
void acc_report()
{
  opFrame[CANcmd] = SWITCH_ACC;
  opFrame[Framelng] = 0x05;
  // Weichenadresse
  opFrame[data0] = 0x00;
  opFrame[data1] = 0x00;
  opFrame[data2] = (uint8_t)(button.Get_to_address() >> 8);
  opFrame[data3] = (uint8_t)button.Get_to_address();
  // Meldung der Lage
  opFrame[data4] = button.GetPosCurr();
  sendCanFrame();
  delay(wait_time); // Delay added just so we can have time to open up
}

// Diese Routine leitet den Positionswechsel einer Weiche/Signal ein.
void switchAcc()
{
  position set_pos = button.GetPosDest();
  switch (set_pos)
  {
  case Upward:
  {
    button.GoUpward();
  }
  break;
  case Down:
  {
    button.GoDown();
  }
  break;
  }
  button.SetPosCurr(set_pos);
  preferences.putUChar("acc_state", (uint8_t)set_pos);
  //
  acc_report();
}

// auf Anforderung des CANguru-Servers sendet der Decoder
// mit dieser Prozedur sendConfig seine Parameterwerte
void sendConfig()
{
  const uint8_t Kanalwidth = 8;
  const uint8_t numberofKanals = endofKanals - 1;
  /*
  Format Gerätebeschreibung
    Unter Index 0 sind die Gerätebeschreibung abrufbar. Primär ist dies die Anzahl der zur Verfügung
    gestellten Messkanäle. Weiterhin enthalten sind Angaben zur Identifikation des Gerätes.
    Format Gerätebeschreibung:
    Typ     Bedeutung
    Char    Anzahl der Messwerte im Gerät.
    Char    Anzahl der Konfigurationskanäle
    2 Byte  frei.
    U32     Seriennummer CS2.
    String  8 Byte Artikelnummer.
    String  Gerätebezeichnung, \0 Terminiert
*/
  const uint8_t NumLinesKanal00 = 5 * Kanalwidth;
  uint8_t arrKanal00[NumLinesKanal00] = {
      /*1*/ Kanal00, numberofKanals, (uint8_t)0, (uint8_t)0, (uint8_t)0, (uint8_t)0, (uint8_t)0, decoderadr,
      /*2.1*/ (uint8_t)highbyte2char(hex2dec(uid_device[0])), (uint8_t)lowbyte2char(hex2dec(uid_device[0])),
      /*2.2*/ (uint8_t)highbyte2char(hex2dec(uid_device[1])), (uint8_t)lowbyte2char(hex2dec(uid_device[1])),
      /*2.3*/ (uint8_t)highbyte2char(hex2dec(uid_device[2])), (uint8_t)lowbyte2char(hex2dec(uid_device[2])),
      /*2.4*/ (uint8_t)highbyte2char(hex2dec(uid_device[3])), (uint8_t)lowbyte2char(hex2dec(uid_device[3])),
      /*3*/ 'C', 'A', 'N', 'g', 'u', 'r', 'u', ' ',
      /*4*/ 'F', 'o', 'r', 'm', 's', 'i', 'g', 'n',
      /*5*/ 'a', 'l', 0, 0, 0, 0, 0, 0};
  /*
  Format eines Datenblocks mit der Möglichkeit einen Wert einzustellen:
    Typ     Bedeutung                   Beispiel
    Char    Konfigirationskanalnummer   0x05: Setzen unter Kanal 5
    Char    Kenner Slider               Wert 2
    Word    Unterer Wert                0
    Word    Oberer Wert                 660
    Word    Aktuelle Einstellung        500
    String  Auswahlbezeichnung          Variable Strombegrenzung\0
    String  Bezeichnung Start           0.000\0
    String  Bezeichnung Ende            2.500\0
    String  Einheit                     Achsen\0 oder A\0

    Bezeichnung Start:
Format einer Gleitkommazahl. Liefert sowohl die Information, für den Startwert der Darstellung als auch
eine Information, wie der Messwert dargestellt werden soll. Die Anzahl der Nachkommastellen sind auch
die Anzahl der Nachkommastellen des Messwerts (mA also 3 Nachkommastellen). Abgefragter
Messwert muss bei der Darstellung durch 1000 geteilt werden. Die Anzahl der Nachkommastellen und
die Potenz des Messwerts müssen identisch sein.
Bezeichnung Ende:
Format und Funktion wie Bezeichnung Start. Für das Ende der Darstellung
*/
  const uint8_t NumLinesKanal01 = 4 * Kanalwidth;
  uint8_t arrKanal01[NumLinesKanal01] = {
      // #2 - WORD immer Big Endian, wie Uhrzeit
      /*1*/ Kanal01, 2, 0, minadr, 0, maxadr, 0, decoderadr,
      /*2*/ 'M', 'o', 'd', 'u', 'l', 'a', 'd', 'r',
      /*3*/ 'e', 's', 's', 'e', 0, '1', 0, (maxadr / 100) + '0',
      /*4*/ (maxadr - (uint8_t)(maxadr / 100) * 100) / 10 + '0', (maxadr - (uint8_t)(maxadr / 10) * 10) + '0', 0, 'A', 'd', 'r', 0, 0};

  const uint8_t NumLinesKanal02 = 5 * Kanalwidth;
  uint8_t arrKanal02[NumLinesKanal02] = {
      /*1*/ Kanal02, 2, 0, minstepperdelay, 0, maxstepperdelay, 0, stepperDelay,
      /*2*/ 'S', 'e', 'r', 'v', 'o', 'v', 'e', 'r',
      /*3*/ 'z', '\xF6', 'g', 'e', 'r', 'u', 'n', 'g',
      /*4*/ 0, minstepperdelay + '0', 0, (maxstepperdelay / 100) + '0', (maxstepperdelay - (uint8_t)(maxstepperdelay / 100) * 100) / 10 + '0', (maxstepperdelay - (uint8_t)(maxstepperdelay / 10) * 10) + '0', 0, 'x',
      /*5*/ 0, 0, 0, 0, 0, 0, 0, 0};

  const uint8_t NumLinesKanal03 = 5 * Kanalwidth;
  uint8_t arrKanal03[NumLinesKanal03] = {
      // Char Konfigirationskanalnummer
      /*1*/ Kanal03,
      // Char Kenner Slider
      2,
      // Word Unterer Wert
      highByte(startPos_min), lowByte(startPos_min),
      // Word Oberer Wert
      highByte(startPos_max), lowByte(startPos_max),
      // Word Aktuelle Einstellung
      highByte(startPos), lowByte(startPos),
      // String Auswahlbezeichnung
      /*2*/ 'S', 't', 'a', 'r', 't', 'p', 'o', 's',
      /*3*/ 'i', 't', 'i', 'o', 'n', 0,
      // String Bezeichnung Start
      '1', 0,
      // String Bezeichnung Ende
      /*4*/ '1', 0,
      // String Einheit
      'S', 't', 'e', 'p', /*5*/ 's', 0, 0, 0, 0, 0, 0, 0};
  const uint8_t NumLinesKanal04 = 4 * Kanalwidth;
  uint8_t arrKanal04[NumLinesKanal04] = {
      // Char Konfigirationskanalnummer
      /*1*/ Kanal04,
      // Char Kenner Slider
      2,
      // Word Unterer Wert
      highByte(endPos_min), lowByte(endPos_min),
      // Word Oberer Wert
      highByte(endPos_max), lowByte(endPos_max),
      // Word Aktuelle Einstellung
      highByte(endPos), lowByte(endPos),
      // String Auswahlbezeichnung
      /*2*/ 'E', 'n', 'd', 'p', 'o', 's', 'i', 't',
      /*3*/ 'i', 'o', 'n', 0,
      // String Bezeichnung Start
      '1', 0,
      // String Bezeichnung Ende
      /*4*/ '1', 0,
      // String Einheit
      'S', 't', 'e', 'p', /*5*/ 's', 0};
  uint8_t NumKanalLines[numberofKanals + 1] = {
      NumLinesKanal00, NumLinesKanal01, NumLinesKanal02, NumLinesKanal03, NumLinesKanal04};

  preferences.putUChar("receiveTheData", true);
  uint8_t paket = 0;
  uint8_t outzeichen = 0;
  CONFIG_Status_Request = false;
  for (uint8_t inzeichen = 0; inzeichen < NumKanalLines[CONFIGURATION_Status_Index]; inzeichen++)
  {
    opFrame[CANcmd] = CONFIG_Status + 1;
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
    case endofKanals:
    {
      // der Vollständigkeit geschuldet
    }
    break;
    }
    outzeichen++;
    if (outzeichen == 8)
    {
      opFrame[Framelng] = 8;
      outzeichen = 0;
      paket++;
      opFrame[hash0] = 0x00;
      opFrame[hash1] = paket;
      sendTheData();
      delay(wait_time_small);
    }
  }
  //
  memset(opFrame, 0, sizeof(opFrame));
  opFrame[CANcmd] = CONFIG_Status + 1;
  opFrame[hash0] = hasharr[0];
  opFrame[hash1] = hasharr[1];
  opFrame[Framelng] = 0x06;
  for (uint8_t i = 0; i < 4; i++)
  {
    opFrame[i + 5] = uid_device[i];
  }
  opFrame[data4] = CONFIGURATION_Status_Index;
  opFrame[data5] = paket;
  sendTheData();
  delay(wait_time_small);
}

// In dieser Schleife verbringt der Decoder die meiste Zeit
void loop()
{
  // die boolsche Variable got1CANmsg zeigt an, ob vom Master
  // eine Nachricht gekommen ist; der Zustand der Flags
  // entscheidet dann, welche Routine anschließend aufgerufen wird
  // die steppers werden permant abgefragt, ob es ein Delta zwischen
  // tatsächlicher und gewünschter stepperstellung gibt
  if (button.Get_set_endpos())
  {
    endPos = button.Get_endpos();
    preferences.putUShort("endPos", endPos);
    //
    button.Reset_endpos();
  }
  button.Update();
  bBlinkAlive = button.is_no_Correction();
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
