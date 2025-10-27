
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
#include <WebServer.h>
#include "Preferences.h"
#include "CANguruDefs.h"
#include <esp_now.h>

#include <Ticker.h>
#include <OTA_include.h>

#include "driver/adc.h"

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

Preferences preferences;

// Forward-Declaration
void sendConfig();
void generateHash(uint8_t offset);
void sendPING();
void sendIP();
void sendCanFrame();

enum boosters
{
  booster_0,
  booster_3,
  booster_6,
  booster_7,
  endbooster
};

adc1_channel_t adc_channels[] = {
    ADC1_CHANNEL_0, // ADC1 channel 0 is GPIO36
    ADC1_CHANNEL_3, // ADC1 channel 3 is GPIO39
    ADC1_CHANNEL_6, // ADC1 channel 6 is GPIO34
    ADC1_CHANNEL_7  // ADC1 channel 7 is GPIO35
};
gpio_num_t enableArr[] = {GPIO_NUM_26, GPIO_NUM_27, GPIO_NUM_14, GPIO_NUM_25};

uint32_t valueArr[] = {0, 0, 0, 0};

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

const uint8_t minAmp = 1;
const uint8_t maxAmp = 20;
const float resistor = 0.68;
uint8_t currMaxAmp;
uint16_t currRAWMaxAmp;

// Protokollkonstanten
#define PROT_MM MM_ACC
#define PROT_DCC DCC_TRACK

IPAddress IP;

void turnPowerOn(boosters booster)
{
  digitalWrite(enableArr[booster], HIGH);
  log_i("Voltage GO - Booster: %d", booster);
}

void turnPowerOff(boosters booster)
{
  digitalWrite(enableArr[booster], LOW);
  log_i("Voltage STOPP - Booster: %d", booster);
}

#include "espnow.h"

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable brownout detector
  Serial.begin(bdrMonitor);
  delay(500);
  Serial.println("\r\n\r\nC A N g u r u - B O O S T E R");
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
    // entspricht 2.0 A
    currMaxAmp = maxAmp;
    preferences.putUChar("maxAmp", currMaxAmp);
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
      currMaxAmp = readValfromPreferences(preferences, "maxAmp", maxAmp, minAmp, maxAmp);
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
  // U = I * R
  log_e("MaxA: %X\r\n", currMaxAmp);
  float volt = float(currMaxAmp) / 10.0 * resistor;
  // I = U / R --> 1.36 Volt / 0.68 Ohm entspricht ca. 2.0 Ampere
  log_e("Volt: %X\r\n", volt);
  currRAWMaxAmp = volt / 3.3 * 4095;
  log_e("currRAWMaxAmp: %X\r\n", currRAWMaxAmp);
  // Variablen werden gemäß der eingelesenen Werte gesetzt
  // enable-Eingänge der Booster einschalten
  // ADC capture width is 12Bit.
  adc1_config_width(ADC_WIDTH_BIT_12);
  for (boosters booster = booster_0; booster < endbooster; booster = boosters(booster + 1))
  {
    pinMode(enableArr[booster], OUTPUT);
    turnPowerOff(booster);
    valueArr[booster] = 0;
    //<The input voltage of ADC will be attenuated extending the range of measurement by about 11 dB (3.55 x)
    adc1_config_channel_atten(adc_channels[booster], ADC_ATTEN_DB_11);
  }
  // Vorbereiten der Blink-LED
  stillAliveBlinkSetup(LED_BUILTIN);
}

// receiveKanalData dient der Parameterübertragung zwischen Decoder und CANguru-Server
// es erhält die evtuelle auf dem Server geänderten Werte zurück
void receiveKanalData()
{
  uint8_t oldval;
  switch (opFrame[10])
  {
  // Kanalnummer #1 - Aktueller Maximalstrom
  case 1:
  {
    oldval = currMaxAmp;
    currMaxAmp = (opFrame[11] << 8) + opFrame[12];
    if (testMinMax(oldval, currMaxAmp, minAmp, maxAmp))
    {
      preferences.putUChar("maxAmp", currMaxAmp);
    }
    else
    {
      currMaxAmp = oldval;
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
      /*1*/ Kanal01, slider, 0, minAmp, 0, maxAmp, 0, currMaxAmp,
      /*     Auswahlbezeichnung */
      /*2*/ 'M', 'a', 'x', ' ', 'S', 't', 'r', 'o',
      /*3*/ 'm', (uint8_t)0,
      /* Bezeichnung Start */
      /*3*/ minAmp / 10 + (uint8_t)'0', '.', minAmp % 10 + (uint8_t)'0', (uint8_t)0,
      /* Bezeichnung Ende */
      /*3*/ maxAmp / 10 + (uint8_t)'0', '.', maxAmp % 10 + (uint8_t)'0', (uint8_t)0,
      /* Einheit */
      /*4*/ 'A', 'm', 'p', (uint8_t)0, (uint8_t)0, (uint8_t)0};

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

void onPrint(boosters booster)
{
  // Subtract min from max
  float voltage_value = (valueArr[booster] * 3.3) / 4095.0; // voltage_value = 1.2;
  uint8_t d0 = (uint8_t)voltage_value;                      // d0 = 1;
  voltage_value -= d0;                                      // voltage_value = 0.2;
  voltage_value *= 10.0;                                    // voltage_value = 2.0;
  uint8_t d1 = (uint8_t)voltage_value;                      // d1 = 2;
  uint8_t index = 2 * booster + 5;
  opFrame[index] = d0;     // 0-5  1-7  2-9  3-11
  opFrame[index + 1] = d1; // 0-6  1-8  2-10  3-12

  opFrame[1] = sendCurrAmp - 1;
  opFrame[4] = 0x08;
  sendCanFrame();
  if (d0 > 0 || d1 > 0)
  {
    log_e("B: %X A:%X.%X\r\n", booster, d0, d1);
  }
}

// In dieser Schleife verbringt der Decoder die meiste Zeit
void loop()
{
  static unsigned long lastUpdate = millis(); // last update of position
  const unsigned long updateInterval = 2000;
  static uint16_t sample[] = {0, 0, 0, 0};
  const uint16_t samples = 2048;
  static boosters booster = endbooster;
  if (bridgeIsConnected2Server)
  {
    booster = boosters(booster + 1);
    if (booster >= endbooster)
      booster = booster_0;
    /*Note: ADC2 pins cannot be used when Wi-Fi is used. So, if you’re using Wi-Fi and you’re having
    trouble getting the value from an ADC2 GPIO, you may consider using an ADC1 GPIO instead, that
    should solve your problem.*/
    // value read from the sensor
    uint16_t currValue = adc1_get_raw(adc_channels[booster]);
    if (currValue > 10)
    {
      valueArr[booster] += currValue;
      sample[booster]++;
    }

    //if ((millis() - lastUpdate) > updateInterval)
    if (sample[booster] >= samples)
    {
      //  if (valueArr[booster]> 10)
      //    log_i("%d: %d Samples: %d - %d", booster, valueArr[booster], sample[booster], valueArr[booster]/sample[booster]);
      sample[booster] = 0;
      valueArr[booster] /= samples;
      if (valueArr[booster] > currRAWMaxAmp)
        turnPowerOff(booster);
      onPrint(booster);
      valueArr[booster] = 0;
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
