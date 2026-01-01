
/* ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <CANguru-Buch@web.de> wrote this file. As long as you retain this
 * notice you can do whatever you want with this stuff. If we meet some day,
 * and you think this stuff is worth it, you can buy me a beer in return
 * Gustav Wostrack
 * ----------------------------------------------------------------------------
 */

#include <WiFi.h>
#include <WiFiClient.h>
#include "Preferences.h"
#include <CANguruDefs.h>
#include <esp_now.h>

#include <Ticker.h>
#include <OTA_include.h>

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "Adafruit_INA3221.h"
#include <Wire.h>

Preferences preferences;
// Create an INA3221 object
Adafruit_INA3221 ina3221;

// Forward-Declaration
void sendConfig();
void generateHash(uint8_t offset);
void sendPING();
void sendIP();
void sendCanFrame();
void turnPowerOn();
void turnPowerOff();

gpio_num_t enableBoosters = GPIO_NUM_20;

// config-Daten
// Parameter-Kanäle
enum Kanals
{
  Kanal00,
  Kanal01,
  endofKanals
};

Kanals CONFIGURATION_Status_Index = Kanal00;

const uint8_t decoderadr = minadr;
uint8_t uid_device[uid_num];
uint16_t ms_nativeDelay;

// Zeigen an, ob eine entsprechende Anforderung eingegangen ist
bool CONFIG_Status_Request = false;
bool SYS_CMD_Request = false;
bool SEND_IP_Request = false;
bool systemIsSeen = false;

boolean statusPING;
boolean bDecoderIsAlive;
boolean bridgeIsConnected2Server;

#define VERS_HIGH 0x00 // Versionsnummer vor dem Punkt
#define VERS_LOW 0x01  // Versionsnummer nach dem Punkt

const uint16_t minAmpLimit_mA = 10;
const uint16_t maxAmpLimit_mA = 1600;
uint16_t currRawMaxAmp_mA;
float currMaxAmp_mA;
float Current_value0_mA = 0.0;
float Current_value1_mA = 0.0;

// Protokollkonstanten
#define PROT_MM MM_ACC
#define PROT_DCC DCC_TRACK

IPAddress IP;

#include "espnow.h"

void turnPowerOn()
{
  digitalWrite(enableBoosters, HIGH);
  // 00 00 47 11 5 00 00 00 00 01 Go an alle
  opFrame[CANcmd] = SYS_CMD;
  opFrame[Framelng] = 0x05;
  opFrame[data4] = SYS_GO;
  opFrame[data5] = 0x00;
  opFrame[data6] = 0x00;
  opFrame[data7] = 0x00;
  sendTheData();
  log_i("Current GO - Booster");
}

void turnPowerOff()
{
  digitalWrite(enableBoosters, LOW);
  // 00 00 47 11 5 00 00 00 00 01 Go an alle
  opFrame[CANcmd] = SYS_CMD;
  opFrame[Framelng] = 0x05;
  opFrame[data4] = SYS_STOPP;
  opFrame[data5] = 0x00;
  opFrame[data6] = 0x00;
  opFrame[data7] = 0x00;
  sendTheData();
  log_i("Current STOPP - Booster");
}

uint8_t beforePoint(float c)
{
  return (uint8_t)c / 100;
}

uint8_t afterPoint(float c)
{
  return (uint8_t)c - beforePoint(c);
}

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable brownout detector
  Serial.begin(bdrMonitor);
  delay(500);
  Serial.println("\r\n\r\nC A N g u r u - B O O S T E R V2.0");
  log_i("\n on %s", ARDUINO_BOARD);
  log_i("CPU Frequency = %d Mhz", F_CPU / 1000000);
  //  log_e("ERROR!");
  //  log_d("VERBOSE");
  //  log_w("WARNING");
  //  log_i("INFO");
  // der Decoder strahlt mit seiner Kennung
  // damit kennt die CANguru-Bridge (der Master) seine Decoder findet
  DEVTYPE = DEVTYPE_CANBOOSTER;
  // startet WLAN im AP-Mode, damit meldet sich der Decoder beim Master
  WiFi.mode(WIFI_OFF); // https://github.com/esp8266/Arduino/issues/3100
                       // Connect to Wi-Fi
  WiFi.mode(WIFI_AP);
  String ssid0 = "CNgrSLV";
  String ssid1 = WiFi.softAPmacAddress();
  ssid0 = ssid0 + ssid1;
  char ssid[30];
  ssid0.toCharArray(ssid, 30);
  WiFi.softAP(ssid);
  WiFi.setTxPower(WIFI_POWER_MINUS_1dBm); // Set power to lowest possible value WIFI_POWER_MINUS_1dBm  WIFI_POWER_19_5dBm
  //
  // der Master (CANguru-Bridge) wird registriert
  addMaster();

  // WLAN -Verbindungen können wieder ausgeschaltet werden
  WiFi.disconnect();

  pinMode(enableBoosters, OUTPUT);
  turnPowerOff();

  // die preferences-Library wird gestartet
  if (preferences.begin(prefName, false))
  {
    log_i("Preferences %s erfolgreich gestartet\r\n", prefName);
  }
  uint8_t setup_todo;
  if (preferences.isKey("setup_done"))
    setup_todo = preferences.getUChar("setup_done", 0xFF);
  else
    log_i("setup_done nicht gefunden! Bitte zunächst Installationsroutine aufrufen!");
  if (setup_todo != setup_done)
  {
    // alles fürs erste Mal
    //
    // wurde das Setup bereits einmal durchgeführt?
    // dann wird dieser Anteil übersprungen
    // 47, weil das EEPROM (hoffentlich) nie ursprünglich diesen Inhalt hatte
    // entspricht 1600 mA
    currRawMaxAmp_mA = maxAmpLimit_mA;
    preferences.putUChar("maxAmpBefore", beforePoint(currRawMaxAmp_mA));
    preferences.putUChar("maxAmpAfter", afterPoint(currRawMaxAmp_mA));
    // ota auf "FALSE" setzen
    preferences.putUChar("ota", startWithoutOTA);
    // setup_done auf "TRUE" setzen
    preferences.putUChar("setup_done", setup_done);
  }
  else
  {
    uint8_t ota = preferences.getUChar("ota", false);
    if (ota == startWithoutOTA)
    {
      // nach dem ersten Mal Einlesen der gespeicherten Werte
      // Adresse
      uint8_t before = 0;
      uint8_t after = 0;
      before = readValfromPreferences(preferences, "maxAmpBefore", before, 0, 1);
      after = readValfromPreferences(preferences, "maxAmpAfter", after, 0, 9);
      currRawMaxAmp_mA = before * 1000 + after * 100;
      currMaxAmp_mA = (float) currRawMaxAmp_mA;
    }
    else
    {
      // ota auf "FALSE" setzen
      preferences.putUChar("ota", startWithoutOTA);
      Connect2WiFiandOTA(preferences);
    }
  }
  // ab hier werden die Anweisungen bei jedem Start durchlaufen
  // IP-Adresse
  IP = readIP(preferences);

  // Flags
  got1CANmsg = false;
  SYS_CMD_Request = false;
  SEND_IP_Request = false;
  statusPING = false;
  bridgeIsConnected2Server = false;
  systemIsSeen = false;
  bDecoderIsAlive = true;
  // Initialize the INA3221
  if (!ina3221.begin(0x40, &Wire))
  { // can use other I2C addresses or buses
    Serial.println("Failed to find INA3221 chip");
    while (1)
      delay(10);
  }
  Serial.println("INA3221 Found!");

  ina3221.setAveragingMode(INA3221_AVG_512_SAMPLES);

  // Set shunt resistances for all channels to 0.05 ohms
  for (uint8_t i = 0; i < 3; i++)
  {
    ina3221.setShuntResistance(i, 0.1);
  }

  // Set a power valid alert to tell us if ALL channels are between the two
  // limits:
  ina3221.setPowerValidLimits(3.0, 15.0); // lower limit - upper limit
  // Variablen werden gemäß der eingelesenen Werte gesetzt
  // enable-Eingänge der Booster einschalten
  // ADC capture width is 12Bit.
  // Vorbereiten der Blink-LED
  stillAliveBlinkSetup(0x00);
}

// receiveKanalData dient der Parameterübertragung zwischen Decoder und CANguru-Server
// es erhält die evtuelle auf dem Server geänderten Werte zurück
void receiveKanalData()
{
  uint16_t oldval;
  switch (opFrame[10])
  {
  // Kanalnummer #1 - Aktueller Maximalstrom
  case 1:
  {
    oldval = currRawMaxAmp_mA;
    currRawMaxAmp_mA = (opFrame[11] << 8) + opFrame[12];
    if (testMinMax(oldval, currRawMaxAmp_mA, minAmpLimit_mA, maxAmpLimit_mA))
    {
      preferences.putUChar("maxAmp", currRawMaxAmp_mA);
    }
    else
    {
      currRawMaxAmp_mA = oldval;
    }
  }
  break;
  }
  //
  opFrame[11] = 0x01;
  opFrame[4] = 0x07;
  sendCanFrame();
}

// auf Anforderung des CANguru-Servers sendet der Decoder
// mit dieser Prozedur sendConfig seine Parameterwerte
void sendConfig()
{
  const uint8_t slider = 2;
  const uint8_t Kanalwidth = 8;
  const uint8_t numberofKanals = endofKanals - 1;

  const uint8_t NumLinesKanal00 = 4 * Kanalwidth;
  uint8_t arrKanal00[NumLinesKanal00] = {
      /*1*/ Kanal00, numberofKanals, (uint8_t)0, (uint8_t)0, (uint8_t)0, (uint8_t)0, (uint8_t)0, decoderadr,
      /*2.1*/ (uint8_t)highbyte2char(hex2dec(uid_device[0])), (uint8_t)lowbyte2char(hex2dec(uid_device[0])),
      /*2.2*/ (uint8_t)highbyte2char(hex2dec(uid_device[1])), (uint8_t)lowbyte2char(hex2dec(uid_device[1])),
      /*2.3*/ (uint8_t)highbyte2char(hex2dec(uid_device[2])), (uint8_t)lowbyte2char(hex2dec(uid_device[2])),
      /*2.4*/ (uint8_t)highbyte2char(hex2dec(uid_device[3])), (uint8_t)lowbyte2char(hex2dec(uid_device[3])),
      /*3*/ 'C', 'A', 'N', 'g', 'u', 'r', 'u', ' ',
      /*4*/ 'B', 'o', 'o', 's', 't', 'e', 'r', (uint8_t)0};

  const uint8_t NumLinesKanal01 = 4 * Kanalwidth;
  uint8_t arrKanal01[NumLinesKanal01] = {
      /*    Konfigirationskanalnummer / Kenner Slider / Unterer Wert (Word) / Oberer Wert (Word) / Aktuelle Einstellung (Word) */
      /*1*/ Kanal01, slider, beforePoint(minAmpLimit_mA), afterPoint(minAmpLimit_mA), beforePoint(maxAmpLimit_mA), afterPoint(maxAmpLimit_mA), beforePoint(currRawMaxAmp_mA), afterPoint(currRawMaxAmp_mA),
      /*     Auswahlbezeichnung */
      /*2*/ 'M', 'a', 'x', ' ', 'S', 't', 'r', 'o',
      /*3*/ 'm', (uint8_t)0,
      /* Bezeichnung Start */
      /*3*/ minAmpLimit_mA / 10 + (uint8_t)'0', '.', minAmpLimit_mA % 10 + (uint8_t)'0', (uint8_t)0,
      /* Bezeichnung Ende */
      /*3*/ maxAmpLimit_mA / 10 + (uint8_t)'0', '.', maxAmpLimit_mA % 10 + (uint8_t)'0', (uint8_t)0,
      /* Einheit */
      /*4*/ 'm', 'A', 'm', 'p', (uint8_t)0, (uint8_t)0};

  uint8_t NumKanalLines[numberofKanals + 1] = {NumLinesKanal00, NumLinesKanal01};

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

void onPrint()
{
  // channel 0
  opFrame[data4] = beforePoint(Current_value0_mA);
  opFrame[data5] = afterPoint(Current_value0_mA);
  // channel 1
  //  Subtract min from max
  opFrame[data6] = beforePoint(Current_value1_mA);
  opFrame[data7] = afterPoint(Current_value1_mA);
  opFrame[CANcmd] = sendCurrAmp - 1;
  opFrame[Framelng] = 0x08;
  log_i("B0:%X.%X - B1:%X.%X", opFrame[data4], opFrame[data5], opFrame[data6], opFrame[data7]);
  sendCanFrame();
}

// In dieser Schleife verbringt der Decoder die meiste Zeit
void loop()
{

  static unsigned long lastUpdate = millis(); // last update of position
  const unsigned long updateInterval = 2000;
  if (bridgeIsConnected2Server)
  {
    // value read from the sensor
    //    uint16_t currValue = adc1_get_raw(adc_channels[booster]);

    if ((millis() - lastUpdate) > updateInterval)
    {
      Current_value0_mA = ina3221.getCurrentAmps(0) * 1000; // Convert to mA
      Current_value1_mA = ina3221.getCurrentAmps(1) * 1000; // Convert to mA
      if ((Current_value0_mA > currMaxAmp_mA) || (Current_value1_mA > currMaxAmp_mA))
      {
        turnPowerOff();
        delay(1000);
      }
      // Display current (in mA) for all channels
      onPrint();
      lastUpdate = millis();
    }
  }
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
  }
}
