
// Import required libraries
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <Preferences.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include "CANguruDefs.h"
#include "OWN_LED.h"
#include "effects.h"

const uint8_t LED_COUNT_NORM = 96; // number of LEDs in the strip

AsyncWebServer server(80);

String LittleFSDateiname = "/cfg_";
String LittleFSDateinameExtra = "/extra_global.json";

Preferences preferences_light;
Preferences preferences_CANguru;

boolean LED_onoff;

#define prefNameLight "strip"
#define setup_done 0x47
#define setup_NOTdone 0xFF

unsigned long lastTime_house; // timestamp in us of when the last step was taken
unsigned long baseTime_house; // delay between steps, in us, based on speed
uint8_t nutzung;
uint8_t effectNbr = 0;

enum statusType
{
  lightOn,
  lightOff
};

struct roomType
{
  uint16_t onTime;
  uint16_t offTime;
  statusType status;
  uint16_t goneTime;
  uint16_t duration;
};
roomType Raum[LED_COUNT_NORM]; // Index 1–96

uint32_t colorOn;
uint32_t colorOff;
uint8_t brightness;

uint8_t waiteffect = 32;

uint8_t currRoom;

enum showType
{
  house = 0,
  effect,
  test,
  do_clearing
};
showType showMode;
showType showModeTmp;
String showModeString;

const uint8_t macLen = 6;
uint8_t nativeMACAddress[macLen];

const gpio_num_t LED_PIN_strip0 = GPIO_NUM_0;
const gpio_num_t LED_PIN_strip1 = GPIO_NUM_1;
const gpio_num_t LED_PIN_strip2 = GPIO_NUM_2;
const gpio_num_t LED_PIN_strip3 = GPIO_NUM_3;
const gpio_num_t LED_PIN_strip4 = GPIO_NUM_4;

uint8_t curr_pin = LED_PIN_strip2;
bool ident;

Adafruit_NeoPixel strip(LED_COUNT_NORM, curr_pin, NEO_RGB + NEO_KHZ800);
uint8_t TestRoom = 0;

const char *hostnameBel = "LICHT";
char hostname[25]; // Enough to hold 3 digits and a null terminator

/**
 * The function `random` generates a random integer within a specified range.
 *
 * @param MaxZahl MaxZahl is the maximum value that the random number should not exceed.
 *
 * @return The function `random` returns a random integer value within the range of 1 to `MaxZahl`,
 * where `MaxZahl` is the maximum value specified as an argument to the function.
 */
int16_t random(int MaxZahl)
{
  float zufall;
  zufall = (float)rand() / RAND_MAX * MaxZahl + 1;
  return ((int16_t)zufall);
}

void speichereInitialKonfigurationen()
{
  // Globale Zusatzwerte einmalig speichern
  if (!LittleFS.exists(LittleFSDateinameExtra))
  {
    File file = LittleFS.open(LittleFSDateinameExtra, "w");
    if (file)
    {
      DynamicJsonDocument extra(128);
      //  JsonDocument extra;
      extra["d4"] = "32";
      extra["d5"] = "32";
      extra["d6"] = "32";
      extra["d7"] = "500";
      extra["d8"] = "24";
      extra["modus"] = "Mode 1"; // Standardwert für Radiobutton
      serializeJson(extra, file);
      file.close();
      Serial.println("Globale Zusatzwerte gespeichert: /extra_global.json");
    }
    else
    {
      Serial.println("Fehler beim Schreiben der globalen Zusatzwerte");
    }
  }

  // Hauptkonfigurationen für alle Indizes
  for (int i = 1; i <= LED_COUNT_NORM; i++)
  {
    String index = String(i);
    String cfgDatei = LittleFSDateiname + index + ".json";

    if (!LittleFS.exists(cfgDatei))
    {
      File file = LittleFS.open(cfgDatei, "w");
      if (file)
      {
        DynamicJsonDocument cfg(256);
        char buffer[20];
        sprintf(buffer, "%d", random(10) + i);
        cfg["d2"] = buffer;
        sprintf(buffer, "%d", random(5) + i);
        cfg["d3"] = buffer;
        serializeJson(cfg, file);
        file.close();
        Serial.println("Hauptkonfiguration gespeichert: " + cfgDatei);
      }
      else
      {
        Serial.println("Fehler beim Schreiben: " + cfgDatei);
      }
    }
    else
    {
      Serial.println("Hauptkonfiguration existiert bereits: " + cfgDatei);
    }
    delay(300);
  }
}

void ladeRaumArray()
{
  for (int i = 0; i < LED_COUNT_NORM; i++)
  {
    String index = String(i);
    String pfad = "/cfg_" + index + ".json";

    File file = LittleFS.open(pfad, "r");
    if (!file)
    {
      Serial.println("Fehlt: " + pfad);
      continue;
    }

    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error)
    {
      Serial.println("Fehler beim Parsen: " + pfad);
      continue;
    }

    Raum[i].onTime = doc["d2"].as<uint16_t>();
    Raum[i].offTime = doc["d3"].as<uint16_t>();
  }
}

void ladeHausKonfiguration()
{
  File file = LittleFS.open("/extra_global.json", "r");
  if (!file)
  {
    Serial.println("Datei /extra_global.json nicht gefunden");
    return;
  }

  DynamicJsonDocument doc(256);
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error)
  {
    Serial.println("Fehler beim Parsen der Zusatzkonfiguration");
    return;
  }

  uint8_t r;
  uint8_t g;
  uint8_t b;
  if (doc.containsKey("d5"))
    r = doc["d5"].as<uint8_t>();
  if (doc.containsKey("d4"))
    g = doc["d4"].as<uint8_t>();
  if (doc.containsKey("d6"))
    b = doc["d6"].as<uint8_t>();
  colorOn = strip.Color(r, g, b);
  colorOff = strip.Color(0, 0, 0);
  if (doc.containsKey("d7"))
    baseTime_house = doc["d7"];
  if (doc.containsKey("d8"))
    nutzung = doc["d8"].as<uint8_t>();
  if (doc.containsKey("modus"))
  {
    showModeString = doc["modus"].as<String>();
    if (showModeString == "Mode 1")
      showMode = house;
    else if (showModeString == "Mode 2")
      showMode = effect;
    else if (showModeString == "Mode 3")
      showMode = test;
    else
      showMode = house; // Default
  }
}

// startet WLAN im AP-Mode, damit meldet sich der Decoder beim Master
void startAPMode()
{
  char hostChar[25]; // Enough to hold 3 digits and a null terminator
  String host = preferences_CANguru.getString("HOST", "No_HOST");
  // Convert String to char array
  host.toCharArray(hostChar, sizeof(hostChar));
  strcpy(hostname, hostnameBel);
  strcat(hostname, hostChar); // Concatenate str2 to str1
  WiFi.setHostname(hostname); // hostnameBel+host);
  WiFi.mode(WIFI_STA);
  WiFi.macAddress(nativeMACAddress);
}

void setup()
{
  char buffer[25]; // Enough to hold 3 digits and a null terminator
  Serial.begin(bdrMonitor);

  Serial.println("\r\n\r\nCANguru - Hausbeleuchtung");
  log_i("\r\n\r\nCANguru - Hausbeleuchtung");
  log_i("\n on %s", ARDUINO_BOARD);
  log_i("CPU Frequency = %d Mhz", F_CPU / 1000000);
  //  log_e("ERROR!");
  //  log_d("VERBOSE");
  //  log_w("WARNING");
  //  log_d("INFO");

  // Dateisystem starten
  if (!LittleFS.begin())
  {
    Serial.println("LittleFS konnte nicht gestartet werden");
    return;
  }

  // die preferences-Library wird gestartet
  if (preferences_CANguru.begin(prefName, false))
  {
    log_d("Preferences_CANguruerfolgreich gestartet");
  }

  // WLAN verbinden
  String ssid;
  ssid = preferences_CANguru.getString("ssid", "No SSID");
  String password;
  password = preferences_CANguru.getString("password", "No password");
  char ssidCh[ssid.length() + 1];
  ssid.toCharArray(ssidCh, ssid.length() + 1);
  char passwordCh[password.length() + 1];
  password.toCharArray(passwordCh, password.length() + 1);
  startAPMode();
  WiFi.begin(ssidCh, passwordCh);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println("\nVerbunden! IP: " + WiFi.localIP().toString());

  if (!MDNS.begin(hostname))
  { // will be available under esp32.local
    Serial.println("Fehler beim Start von mDNS");
    while (1)
      delay(500); // stop
  }
  Serial.println("mDNS gestartet: " + String(hostname) + ".local");
  ident = false;
  IPAddress ip = WiFi.localIP();
  preferences_CANguru.end();

  // -----------------------------------------------------------------> HTML-Seite ausliefern

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/index.html", "text/html"); });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/style.css", "application/javascript"); });

  // -----------------------------------------------------------------> IP-Adresse ermitteln und an die HTML-Seite schickenn
  server.on("/ip", HTTP_GET, [](AsyncWebServerRequest *request)
            {
  String ip = WiFi.localIP().toString();
  request->send(200, "text/plain", ip); });

  // -----------------------------------------------------------------> Feste Werte laden und aan die HTML-Seite liefern
  server.on("/konfig", HTTP_GET, [](AsyncWebServerRequest *request)
            {
  String index = request->getParam("index")->value();
  File file = LittleFS.open(LittleFSDateiname + index + ".json", "r");
  if (!file) {
    request->send(404, "application/json", "{}");
    return;
  }
  String json = file.readString();
  file.close();
  Serial.println("--> Laden - konfig: " + json);
  request->send(200, "application/json", json); });

  // -----------------------------------------------------------------> Zusatzwerte laden und aan die HTML-Seite liefern
  server.on("/extra", HTTP_GET, [](AsyncWebServerRequest *request)
            {
  File file = LittleFS.open(LittleFSDateinameExtra, "r");
  if (!file) {
    request->send(404, "application/json", "{}");
    return;
  }
  String json = file.readString();
  file.close();
  Serial.println("--> Laden - extra: " + json);
  request->send(200, "application/json", json); });

  // -----------------------------------------------------------------> Neue Konfiguration für Feld 1 bis 6 empfangen und speichern
  server.on("/speichern", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
            {
  String body = String((char*)data).substring(0, len);
  DynamicJsonDocument doc(512);
  if (deserializeJson(doc, body)) {
    request->send(400, "text/plain", "Ungültiges JSON");
    return;
  }

  String KonfigurationsIndexString = doc["d1"].as<String>();
  uint8_t KonfigurationsIndexint = doc["d1"].as<uint8_t>()-1;
  doc.remove("d1");

  File file = LittleFS.open(LittleFSDateiname + KonfigurationsIndexString + ".json", "w");
  if (!file) {
    request->send(500, "text/plain", "Fehler beim Schreiben");
    return;
  }
    Raum[KonfigurationsIndexint].onTime = doc["d2"].as<uint16_t>();
    Raum[KonfigurationsIndexint].offTime = doc["d3"].as<uint16_t>();
    Raum[KonfigurationsIndexint].goneTime = 0;
    Raum[KonfigurationsIndexint].duration = Raum[KonfigurationsIndexint].onTime + Raum[KonfigurationsIndexint].offTime;

  serializeJson(doc, file);
  file.close();
  Serial.println("--> Speichern - konfig: " + doc.as<String>());
  request->send(200, "text/plain", "Hauptkonfiguration gespeichert"); });

  // -----------------------------------------------------------------> Neue Konfiguration für Feld 7 und 8 empfangen und speichern
  server.on("/speichern_extra", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
            {
  String body = String((char*)data).substring(0, len);
  DynamicJsonDocument doc(256);
  if (deserializeJson(doc, body)) {
    request->send(400, "text/plain", "Ungültiges JSON");
    return;
  }

  File file = LittleFS.open(LittleFSDateinameExtra, "w");
  if (!file) {
    request->send(500, "text/plain", "Fehler beim Schreiben");
    return;
  }
uint8_t r;
uint8_t g;
uint8_t b;
  if (doc.containsKey("d5")) r = doc["d5"].as<uint8_t>();
  if (doc.containsKey("d4")) g = doc["d4"].as<uint8_t>();
  if (doc.containsKey("d6")) b = doc["d6"].as<uint8_t>();
  colorOn = strip.Color(r, g, b);
  colorOff = strip.Color(0, 0, 0);
  if (doc.containsKey("d7")) baseTime_house = doc["d7"];
  if (doc.containsKey("d8")) nutzung = doc["d8"].as<uint8_t>();

  // alten Modus zwischenspeichern   
  String showModeStringOld = showModeString;
  if (doc.containsKey("modus")) showModeString = doc["modus"].as<String>();
  if (showModeString != showModeStringOld)
    {
      Serial.println("showmode: " + showModeStringOld + " -> " + showModeString);
      // alten Modus zwischenspeichern
      if (showModeString == "Mode 1") showModeTmp = house;
      else if (showModeString == "Mode 2") showModeTmp = effect;
      else if (showModeString == "Mode 3") showModeTmp = test;
      else showModeTmp = house; // Default 
      showMode = do_clearing; // set to pause first to avoid issues during mode change
                              // wenn der Modus sich ändert, alle LEDs aus
    }

  serializeJson(doc, file);
  file.close();
  Serial.println("--> Speichern - extra: " + doc.as<String>());
  request->send(200, "text/plain", "Globale Zusatzwerte gespeichert"); });

  // -----------------------------------------------------------------> Ende der HTML-Prozeduren
  server.begin();

  MDNS.addService("http", "tcp", 80);

  if (preferences_light.begin(prefNameLight, false))
  {
    log_d("Preferences erfolgreich gestartet");
  }
  uint8_t setup_todo = preferences_light.getUChar("setup_done", 0xFF);
  if (setup_todo != setup_done)
  {
    randomSeed(nativeMACAddress[0] + nativeMACAddress[1] + nativeMACAddress[2] + nativeMACAddress[3] + nativeMACAddress[4] + nativeMACAddress[5]);
    speichereInitialKonfigurationen(); // Nur beim ersten Start!
    // setup_done auf "TRUE" setzen
    preferences_light.putUChar("setup_done", setup_done);
  }
  ladeRaumArray();
  ladeHausKonfiguration();
  for (uint8_t r = 0; r < strip.numPixels(); r++)
  {
    Raum[r].status = lightOff;
    Raum[r].goneTime = 0;
    Raum[r].duration = Raum[r].onTime + Raum[r].offTime;
    strip.setPixelColor(r, colorOff);
  }
  ownshow();
  LED_onoff = true;
  LED_begin(GPIO_NUM_8);
}

void loop()
{
  switch (showMode)
  {
  case house:
    // Hausbeleuchtung
    //    Serial.println("HAUS");
    if (millis() - lastTime_house >= baseTime_house)
    {
      lastTime_house = millis();
      // get the timeStamp of when you stepped:
      for (uint8_t r = 0; r < nutzung; r++)
      {
        Raum[r].goneTime++;
        if (Raum[r].goneTime > Raum[r].duration)
        {
          Raum[r].goneTime = 0;
          Raum[r].status = lightOff;
        }
        if (Raum[r].goneTime > Raum[r].offTime)
        {
          Raum[r].status = lightOn;
        }
        if (Raum[r].status == lightOn)
          strip.setPixelColor(r, colorOn);
        else
          strip.setPixelColor(r, colorOff);
        ownshow();
      }
    }
    break;
  case effect:
    //    Serial.println("Effekt");
    switch (effectNbr)
    {
    case 0:
      //      Serial.println("colorWipe");
      colorWipe(strip.Color(255, 0, 0), waiteffect); // Red
      break;
    case 1:
      colorWipe(strip.Color(0, 255, 0), waiteffect); // Green
      break;
    case 2:
      colorWipe(strip.Color(0, 0, 255), waiteffect); // Blue
      break;
    case 3:
      colorWipe(strip.Color(0, 0, 0, 255), waiteffect); // White RGBW
      break;
    case 4:
      // Send a theater pixel chase in...
      //      Serial.println("theaterChase White");
      theaterChase(strip.Color(127, 127, 127), waiteffect); // White
      break;
    case 5:
      //      Serial.println("theaterChase Red");
      theaterChase(strip.Color(127, 0, 0), waiteffect); // Red
      break;
    case 6:
      //      Serial.println("theaterChase Blue");
      theaterChase(strip.Color(0, 0, 127), waiteffect); // Blue
      break;
    case 7:
      //      Serial.println("rainbow");
      rainbow(20);
      break;
    case 8:
      //      Serial.println("rainbowCycle");
      rainbowCycle(20);
      break;
    case 9:
      //      Serial.println("theaterChaseRainbow");
      theaterChaseRainbow(waiteffect);
      break;
    case 10:
      //      Serial.println("whiteOverRainbow");
      whiteOverRainbow(75, 5);
      break;
    case 11:
      //      Serial.println("pulseWhite");
      pulseWhite(5, 0, 255);
      break;
    case 12:
      //      Serial.println("rainbowFade2White");
      rainbowFade2White(3, 3, 1);
      break;
    }
    effectNbr++;
    if (effectNbr > 12)
      effectNbr = 0;
    break;
  case test:
    //    Serial.println("Test");
    // TEST
    if (millis() - lastTime_house >= baseTime_house / 2)
    {
      lastTime_house = millis();
      //
      if (LED_onoff)
      {
        LED_on();
        strip.setPixelColor(TestRoom, colorOn);
      }
      else
      {
        LED_off();
        TestRoom++;
        if (TestRoom >= nutzung)
        {
          delay(baseTime_house * 4);
          for (uint8_t l = 0; l < nutzung; l++)
          {
            strip.setPixelColor(l, colorOff);
          }
          ownshow();
          TestRoom = 0;
        }
      }
      ownshow();
      LED_onoff = !LED_onoff;
    }
    break;
  case do_clearing:
    // do nothing
    own_clear();
    showMode = showModeTmp;
    //  yield();
    break;

  default:
    break;
  }
  //
}