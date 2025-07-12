#include <Arduino.h>

#include <Adafruit_NeoPixel.h>
#include "preferences.h"

// Import required libraries
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "LittleFS.h"
#include <ESPmDNS.h>
#include "CANguruDefs.h"
#include "OWN_LED.h"

const char *hostnameBel = "LICHT";
char hostname[25]; // Enough to hold 3 digits and a null terminator

Preferences preferences_light;
Preferences preferences_CANguru;

// Replace with your network credentials

const gpio_num_t LED_PIN_HOUSELIGHT0 = GPIO_NUM_0;
const gpio_num_t LED_PIN_HOUSELIGHT1 = GPIO_NUM_1;
const gpio_num_t LED_PIN_HOUSELIGHT2 = GPIO_NUM_2;
const gpio_num_t LED_PIN_HOUSELIGHT3 = GPIO_NUM_3;
const gpio_num_t LED_PIN_HOUSELIGHT4 = GPIO_NUM_4;

#define LED_COUNT 24
// limits für Raum
const uint8_t raum_min = 0;
const uint8_t raum_max = LED_COUNT - 1;
const uint8_t raum_step = 1;
const uint8_t raum_curr = 1;
// limits für onTime
const uint8_t onTime_min = 1;
const uint8_t onTime_max = 25;
const uint8_t onTime_step = 1;
const uint8_t onTime_curr = 10;
// limits für offTime
const uint8_t offTime_min = 1;
const uint8_t offTime_max = 25;
const uint8_t offTime_step = 1;
const uint8_t offTime_curr = 10;
// limits für Brightness
const uint8_t brightness_min = 0;
const uint8_t brightness_max = 255;
const uint8_t brightness_step = 4;
const uint8_t brightness_curr = 32;
// limits für Zeitfaktor
const uint16_t zeitfaktor_min = 500;
const uint16_t zeitfaktor_max = 50000;
const uint16_t zeitfaktor_step = 500;
const uint16_t zeitfaktor_curr = 5000;
// limits für Nutzung
const uint8_t nutzung_min = raum_min + 1;
const uint8_t nutzung_max = raum_max + 1;
const uint8_t nutzung_step = raum_step;
const uint8_t nutzung_curr = nutzung_max;

uint8_t curr_pin = LED_PIN_HOUSELIGHT2;

Adafruit_NeoPixel houselight(LED_COUNT, curr_pin, NEO_RGB + NEO_KHZ800);

boolean LED_onoff;

#define prefNameLight "HOUSELIGHT"
#define setup_done 0x47
#define setup_NOTdone 0xFF

unsigned long lastTime_house; // timestamp in us of when the last step was taken
unsigned long baseTime_house; // delay between steps, in us, based on speed
uint8_t nutzung;

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
roomType rooms[LED_COUNT];

uint8_t currRoom;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Replaces placeholder with LED state value
String processor(const String &var)
{
  return String();
}

int16_t random(int MaxZahl)
{
  float zufall;
  zufall = (float)rand() / RAND_MAX * MaxZahl + 1;
  return ((int16_t)zufall);
}

//////////////////////////////////////////

// Teilt einen String mit Hilfe des Separators in seine Bestandteile auf
// und gibt den index-Teil zurück
String getStringPartByNr(String data, char separator, int index)
{
  int stringData = 0;   // variable to count data part nr
  String dataPart = ""; // variable to hole the return text

  for (int i = 0; i < data.length() - 1; i++)
  { // Walk through the text one letter at a time
    if (data[i] == separator)
    {
      // Count the number of times separator character appears in the text
      stringData++;
    }
    else if (stringData == index)
    {
      // get the text when separator is the rignt one
      dataPart.concat(data[i]);
    }
    else if (stringData > index)
    {
      // return text and stop if the next separator appears - to save CPU-time
      return dataPart;
      break;
    }
  }
  // return text if this is the last part
  return dataPart;
}

void onRequest(AsyncWebServerRequest *request)
{
  String urlLine = request->url();
  String val;
  char buffer[25]; // Enough to hold 3 digits and a null terminator
  if (urlLine.indexOf("/SETPARAMS") >= 0)
  {
    // die aktuellen Werte aus der HTML-Seite werden von dort hierhin übertragen
    // und nun hier auf dem ESP32C3 gespeichert
    // dieser Prozess wird auf der HTML-Seite durch Knopfdruck SPEICHERN ausgelöst
    Serial.println("URL: " + String(urlLine));
    for (uint8_t p = 0; p < 6; p++)
    {
      // beginne mit dem 2. Parameter (ohne /PARAM)
      val = getStringPartByNr(urlLine, '/', p + 2);
      switch (p)
      {
      case 0:
        // aktueller Raum
        currRoom = (uint8_t)val.toInt();
        preferences_light.putUShort("currRoom", currRoom);
        break;
      case 1:
        // onTime-Wert v1 für die Raumnummer aus v0
        rooms[currRoom].onTime = (uint8_t)val.toInt();
        sprintf(buffer, "onTime%d", currRoom);
        preferences_light.putUShort(buffer, rooms[currRoom].onTime);
        break;
      case 2:
        // offTime-Wert v2 für die Raumnummer aus v0
        rooms[currRoom].offTime = (uint8_t)val.toInt();
        sprintf(buffer, "offTime%d", currRoom);
        preferences_light.putUShort(buffer, rooms[currRoom].offTime);
        //      EEPROM.writeByte(eepromAdrParameter2, reflowPWM);
        break;
      case 3:
        // brightness-Wert V3 für den Raum aus v0
        rooms[currRoom].brightness = (uint8_t)val.toInt();
        sprintf(buffer, "brightness%d", currRoom);
        preferences_light.putUShort(buffer, rooms[currRoom].brightness);
        break;
      case 4:
        // Zeitfaktor-Wert V4 für alle Räume
        baseTime_house = (uint16_t)val.toInt();
        preferences_light.putUShort("baseTime", baseTime_house);
        break;
      case 5:
        // Nutzungs-Wert (genutzte LED) V5 für alle Räume
        // zunächst alles auf dunkel
        for (uint8_t l = 0; l < nutzung; l++)
        {
          houselight.setPixelColor(l, rooms[l].colorOff);
          houselight.show();
        }

        nutzung = (uint16_t)val.toInt();
        preferences_light.putUShort("nutzung", nutzung);
        break;
      }
      //      esp_restart();
      rooms[currRoom].colorOn = houselight.Color(rooms[currRoom].brightness, rooms[currRoom].brightness, rooms[currRoom].brightness);
    }
  }
  if (urlLine.indexOf("/CHGRAUM") >= 0)
  {
    // die aktuelle Raumnummer hat gewechselt
    currRoom = getStringPartByNr(urlLine, '/', 2).toInt(); // Raum
    sprintf(buffer, "onTime%d", currRoom);
    rooms[currRoom].onTime = preferences_light.getUShort(buffer, false);
    sprintf(buffer, "offTime%d", currRoom);
    rooms[currRoom].offTime = preferences_light.getUShort(buffer, false);
    sprintf(buffer, "brightness%d", currRoom);
    rooms[currRoom].brightness = preferences_light.getUShort(buffer, false);
    // unchangeable values
    rooms[currRoom].colorOn = houselight.Color(rooms[currRoom].brightness, rooms[currRoom].brightness, rooms[currRoom].brightness);
    rooms[currRoom].colorOff = houselight.Color(0, 0, 0);
    rooms[currRoom].status = lightOff;
    rooms[currRoom].goneTime = 0;
    rooms[currRoom].duration = rooms[currRoom].onTime + rooms[currRoom].offTime;
    houselight.setPixelColor(currRoom, houselight.Color(0, 0, 0));
  }
}

// Diese Funktion wird von der Browseranwendung direkt zu Beginn aufgeruen und
// gibt zurück, die Gesamtzeit eines Temperaturverlaufes, sowie die PWM-Werte der
// einzelnen Phasen.
String IPNbr()
{
  String ipString = WiFi.localIP().toString().c_str();
  return ipString;
}

String GetParams()
{
  uint8_t roomnbr = currRoom;
  String paramsString = String(rooms[roomnbr].onTime) + "/" + String(rooms[roomnbr].offTime) + "/" + String(rooms[roomnbr].brightness) + "/" + String(baseTime_house) + "/" + String(nutzung) + "/";
  return paramsString;
}

String RaumLimits()
{
  String paramsString = String(raum_min) + "/" + String(raum_max) + "/" + String(raum_step) + "/" + String(raum_curr);
  return paramsString;
}

String onTimeLimits()
{
  String paramsString = String(onTime_min) + "/" + String(onTime_max) + "/" + String(onTime_step) + "/" + String(onTime_curr);
  return paramsString;
}

String offTimeLimits()
{
  String paramsString = String(offTime_min) + "/" + String(offTime_max) + "/" + String(offTime_step) + "/" + String(offTime_curr);
  return paramsString;
}

String brightnessLimits()
{
  String paramsString = String(brightness_min) + "/" + String(brightness_max) + "/" + String(brightness_step) + "/" + String(brightness_curr);
  return paramsString;
}

String zeitfaktorLimits()
{
  String paramsString = String(zeitfaktor_min) + "/" + String(zeitfaktor_max) + "/" + String(zeitfaktor_step) + "/" + String(zeitfaktor_curr);
  return paramsString;
}

String nutzungLimits()
{
  String paramsString = String(nutzung_min) + "/" + String(nutzung_max) + "/" + String(nutzung_step) + "/" + String(nutzung_curr);
  return paramsString;
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
}

void setup()
{
  char buffer[25]; // Enough to hold 3 digits and a null terminator
  Serial.begin(bdrMonitor);
  ///
  Serial.println("\r\n\r\nCANguru - Hausbeleuchtung");
  log_i("\r\n\r\nCANguru - Hausbeleuchtung");
  log_i("\n on %s", ARDUINO_BOARD);
  log_i("CPU Frequency = %d Mhz", F_CPU / 1000000);
  //  log_e("ERROR!");
  //  log_d("VERBOSE");
  //  log_w("WARNING");
  //  log_d("INFO");

  // Initialize LittleFS
  if (!LittleFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }

  // die preferences-Library wird gestartet
  if (preferences_CANguru.begin(prefName, false))
  {
    log_d("Preferences_CANguruerfolgreich gestartet");
  }

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

  if (!MDNS.begin(hostname))
  { // will be available under esp32.local
    Serial.println("Fehler beim Start von mDNS");
    while (1)
      delay(500); // stop
  }
  Serial.println("mDNS gestartet. ");

  IPAddress ip = WiFi.localIP();
  // Print ESP32 Local IP Address
  Serial.println("IP:" + String(ip));

  preferences_CANguru.end();
  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/index.html", String(), false, processor); });

  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/style.css", "text/css"); });

  server.on("/IPNBR", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", IPNbr().c_str()); });

  server.on("/GETPARAMS", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", GetParams().c_str()); });

  server.on("/Raum", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", RaumLimits().c_str()); });

  server.on("/onTime", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", onTimeLimits().c_str()); });

  server.on("/offTime", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", offTimeLimits().c_str()); });

  server.on("/brightness", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", brightnessLimits().c_str()); });

  server.on("/zeitfaktor", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", zeitfaktorLimits().c_str()); });

  server.on("/nutzung", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", nutzungLimits().c_str()); });

  // Start server
  server.onNotFound(onRequest);
  server.begin();

  houselight.begin();
  // die preferences-Library wird gestartet

  if (preferences_light.begin(prefNameLight, false))
  {
    log_d("Preferences erfolgreich gestartet");
  }
  uint8_t setup_todo = preferences_light.getUChar("setup_done", 0xFF);
  if (setup_todo != setup_done)
  {
    for (uint8_t r = 0; r < houselight.numPixels(); r++)
    {
      // changeable values
      // onTime
      rooms[r].onTime = random(10) + r;
      sprintf(buffer, "onTime%d", r);
      Serial.println("PUT: " + String(buffer));
      preferences_light.putUShort(buffer, rooms[r].onTime);
      // offTime
      rooms[r].offTime = random(5) + 2 * r;
      sprintf(buffer, "offTime%d", r);
      preferences_light.putUShort(buffer, rooms[r].offTime);
      // brightness
      rooms[r].brightness = brightness_curr;
      sprintf(buffer, "brightness%d", r);
      preferences_light.putUShort(buffer, rooms[r].brightness);
      // unchangeable values
      rooms[r].colorOn = houselight.Color(rooms[r].brightness, rooms[r].brightness, rooms[r].brightness);
      rooms[r].colorOff = houselight.Color(0, 0, 0);
      rooms[r].status = lightOff;
      rooms[r].goneTime = 0;
      rooms[r].duration = rooms[r].onTime + rooms[r].offTime;
      houselight.setPixelColor(r, houselight.Color(0, 0, 0));
    }
    baseTime_house = zeitfaktor_curr; // delay between steps, in us, based on speed
    preferences_light.putUShort("baseTime", baseTime_house);
    nutzung = nutzung_curr; // delay between steps, in us, based on speed
    preferences_light.putUShort("nutzung", nutzung);
    currRoom = raum_curr;
    preferences_light.putUShort("currRoom", currRoom);
    // setup_done auf "TRUE" setzen
    preferences_light.putUChar("setup_done", setup_done);
  }
  else
  {
    for (uint8_t r = 0; r < houselight.numPixels(); r++)
    {
      // changeable values
      sprintf(buffer, "onTime%d", r);
      Serial.println("GET: " + String(buffer));
      rooms[r].onTime = preferences_light.getUShort(buffer, false);
      sprintf(buffer, "offTime%d", r);
      rooms[r].offTime = preferences_light.getUShort(buffer, false);
      sprintf(buffer, "brightness%d", r);
      rooms[r].brightness = preferences_light.getUShort(buffer, false);
      // unchangeable values
      rooms[r].colorOn = houselight.Color(rooms[r].brightness, rooms[r].brightness, rooms[r].brightness);
      rooms[r].colorOff = houselight.Color(0, 0, 0);
      rooms[r].status = lightOff;
      rooms[r].goneTime = 0;
      rooms[r].duration = rooms[r].onTime + rooms[r].offTime;
      houselight.setPixelColor(r, houselight.Color(0, 0, 0));
    }
    baseTime_house = preferences_light.getUShort("baseTime", baseTime_house);
    nutzung = preferences_light.getUShort("nutzung", nutzung);
    currRoom = preferences_light.getUShort("currRoom", 0);
  }
  houselight.show();
  LED_onoff = true;
  LED_begin(GPIO_NUM_8);
}

void loop()
{
  // Hausbeleuchtung
  if (millis() - lastTime_house >= baseTime_house)
  {
    if (LED_onoff)
      LED_on();
    else
      LED_off();
    LED_onoff = !LED_onoff;
    // get the timeStamp of when you stepped:
    lastTime_house = millis();
    for (uint8_t r = 0; r < nutzung; r++)
    {
      rooms[r].goneTime++;
      if (rooms[r].goneTime > rooms[r].duration)
      {
        rooms[r].goneTime = 0;
        rooms[r].status = lightOff;
      }
      if (rooms[r].goneTime > rooms[r].offTime)
      {
        rooms[r].status = lightOn;
      }
      if (rooms[r].status == lightOn)
        houselight.setPixelColor(r, rooms[r].colorOn);
      else
        houselight.setPixelColor(r, rooms[r].colorOff);
      houselight.show();
    }
  }
}
