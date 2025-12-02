
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
  uint32_t colorOn;
  uint32_t colorOff;
  uint8_t brightness;
  statusType status;
  uint16_t goneTime;
  uint16_t duration;
};
// rooms[Raum]
roomType rooms[LED_COUNT_NORM];

uint8_t currRoom;

enum showType
{
  house = 0,
  effect,
  test,
  do_clearing
};
showType showMode = house;
showType showModeTmp;

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
  for (int16_t i = 1; i <= LED_COUNT_NORM; i++)
  {
    String dateiname = LittleFSDateiname + String(i) + ".json";
    if (!LittleFS.exists(dateiname))
    {
      DynamicJsonDocument doc(256);
      char buffer[20];
      sprintf(buffer, "%d", random(10) + i);
      doc["d2"] = buffer; // onTime
      sprintf(buffer, "%d", random(5) + i);
      doc["d3"] = buffer;                 // offTime
      doc["d4"] = "32";
      doc["d5"] = "32";
      doc["d6"] = "32";
      doc["d7"] = "500";
      doc["d8"] = "24";
      doc["mode"] = "Mmde 1";
      File file = LittleFS.open(dateiname, "w");
      if (file)
      {
        serializeJson(doc, file);
        file.close();
        Serial.println("Gespeichert: " + dateiname);
      }
      else
      {
        Serial.println("Fehler beim Schreiben: " + dateiname);
      }
    }
  }
}

String ladeKonfiguration(String index)
{
  String dateiname = LittleFSDateiname + index + ".json";
  if (!LittleFS.exists(dateiname))
    return "{}";

  File file = LittleFS.open(dateiname, "r");
  if (!file)
    return "{}";

  String json = file.readString();
  file.close();
  return json;
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
  Serial.println("mDNS gestartet: "+String(hostname)+".local");
  ident = false;
  IPAddress ip = WiFi.localIP();
//  preferences_CANguru.end();

  // HTML-Seite ausliefern
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/index.html", "text/html"); });

  server.on("/konfig", HTTP_GET, [](AsyncWebServerRequest *request)
            {
  if (!request->hasParam("index")) {
    request->send(400, "text/plain", "Fehlender Index");
    return;
  }
  String index = request->getParam("index")->value();
  String json = ladeKonfiguration(index);
  request->send(200, "application/json", json); });

  // Neue Konfiguration empfangen und speichern
  server.on("/speichern", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
            {
  String body = String((char*)data).substring(0, len);

  DynamicJsonDocument doc(512);
  DeserializationError error = deserializeJson(doc, body);
  if (error) {
    request->send(400, "text/plain", "Ungültiges JSON");
    return;
  }

  if (!doc.containsKey("d1")) {
    request->send(400, "text/plain", "Feld d1 fehlt");
    return;
  }

  String KonfigIndex = doc["d1"].as<String>();
  doc.remove("d1"); // nicht speichern

  String dateiname = "/cfg_" + KonfigIndex + ".json";
  File file = LittleFS.open(dateiname, "w");
  if (!file) {
    request->send(500, "text/plain", "Fehler beim Schreiben");
    return;
  }

  serializeJson(doc, file);
  file.close();
  request->send(200, "text/plain", "Konfiguration gespeichert für d1 = " + KonfigIndex); });

  // IP-Adresse ermitteln und an die HTML-Seite schickenn
  server.on("/ip", HTTP_GET, [](AsyncWebServerRequest *request)
            {
  String ip = WiFi.localIP().toString();
  request->send(200, "text/plain", ip); });
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
    for (uint8_t r = 0; r < strip.numPixels(); r++)
    {
      rooms[r].brightness = preferences_light.getUShort(buffer, false);
      // unchangeable values
      rooms[r].colorOn = strip.Color(rooms[r].brightness, rooms[r].brightness, rooms[r].brightness);
      rooms[r].colorOff = strip.Color(0, 0, 0);
      rooms[r].status = lightOff;
      rooms[r].goneTime = 0;
      rooms[r].duration = rooms[r].onTime + rooms[r].offTime;
      strip.setPixelColor(r, strip.Color(0, 0, 0));
    }
    baseTime_house = preferences_light.getUShort("baseTime", baseTime_house);
    nutzung = preferences_light.getUShort("nutzung", nutzung);
    currRoom = preferences_light.getUShort("currRoom", 0);
    showMode = (showType)preferences_light.getUShort("showMode", house);
  ownshow();
  LED_onoff = true;
  LED_begin(GPIO_NUM_8);
}

void loop()
{
  // Nichts zu tun hier
}