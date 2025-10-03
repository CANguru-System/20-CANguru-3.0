#include <Arduino.h>

// Import required libraries
#include <Adafruit_NeoPixel.h>
#include "preferences.h"
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "LittleFS.h"
#include <ESPmDNS.h>
#include "CANguruDefs.h"
#include "OWN_LED.h"
#include "effects.h"

const uint8_t LED_COUNT_NORM = 96; // number of LEDs in the strip

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

const uint8_t macLen = 6;
uint8_t nativeMACAddress[macLen];

Preferences preferences_light;
Preferences preferences_CANguru;

// limits für Raum
const uint8_t raum_min = 0;
const uint8_t raum_max = LED_COUNT_NORM - 1;
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
const uint8_t nutzung_min = 4;
const uint8_t nutzung_max = LED_COUNT_NORM - 1;
const uint8_t nutzung_step = nutzung_min;
const uint8_t nutzung_curr = 24;

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
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Replaces placeholder with LED state value
String processor(const String &var)
{
  return String();
}

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

//////////////////////////////////////////

// Teilt einen String mit Hilfe des Separators in seine Bestandteile auf
// und gibt den index-Teil zurück
/**
 * The function `getStringPartByNr` extracts a specific part of a string based on a given separator and
 * index.
 *
 * @param data The `data` parameter is the string from which you want to extract a specific part based
 * on the separator and index provided.
 * @param separator The `separator` parameter in the `getStringPartByNr` function is the character that
 * is used to separate different parts of the input `data` string. The function aims to extract a
 * specific part of the string based on the index provided, using the separator to identify different
 * parts.
 * @param index The `index` parameter in the `getStringPartByNr` function represents the position of
 * the data part you want to extract from the input string. It is used to specify which part of the
 * string, separated by the given separator character, you are interested in retrieving.
 *
 * @return The function is intended to return a specific part of a string based on a given separator
 * and index. However, there are some issues in the implementation that need to be corrected.
 */
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

/**
 * The function `onRequest` processes incoming requests to set parameters or change room settings in an
 * ESP32C3 device based on the URL received.
 *
 * @param request The `onRequest` function you provided is a part of a web server implementation on an
 * ESP32C3 microcontroller. This function handles incoming HTTP requests and processes the parameters
 * sent in the URL to update certain settings related to lighting control in different rooms.
 */
void onRequest(AsyncWebServerRequest *request)
{
  String urlLine = request->url();
  String val;
  char buffer[25]; // Enough to hold 3 digits and a null terminator
  showType showModeOld;
  Serial.println("URL: " + String(urlLine));
  if (urlLine.indexOf("/IDENT") >= 0)
  {
    request->send_P(200, "text/plain", "ok");
    ident = true;
  }
  if (urlLine.indexOf("/SETPARAMS") >= 0)
  {
    // die aktuellen Werte aus der HTML-Seite werden von dort hierhin übertragen
    // und nun hier auf dem ESP32C3 gespeichert
    // dieser Prozess wird auf der HTML-Seite durch Knopfdruck SPEICHERN ausgelöst
    for (uint8_t p = 0; p < 7; p++)
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
        nutzung = (uint16_t)val.toInt();
        preferences_light.putUShort("nutzung", nutzung);
        break;
      case 6:
        // Effekt-Wert
        showModeOld = showMode;
        showModeTmp = (showType)val.toInt();
        if (showModeTmp != showModeOld)
        {
          Serial.println("showmode: " + String(showModeOld) + " -> " + String(showMode));
        showMode = do_clearing; // set to pause first to avoid issues during mode change
          // wenn der Modus sich ändert, alle LEDs aus
        }
        preferences_light.putUShort("showMode", showMode);
        break;
      }
      //      esp_restart();
      //      rooms[currRoom].colorOn = strip.Color(rooms[currRoom].brightness, rooms[currRoom].brightness, rooms[currRoom].brightness);
    }
  }
  if (urlLine.indexOf("/CHGRAUM") >= 0)
  {
    // die aktuelle Raumnummer hat gewechselt, alle Werte dieses Raumes werden geladen
    currRoom = getStringPartByNr(urlLine, '/', 2).toInt(); // Raum
    sprintf(buffer, "onTime%d", currRoom);
    rooms[currRoom].onTime = preferences_light.getUShort(buffer, false);
    sprintf(buffer, "offTime%d", currRoom);
    rooms[currRoom].offTime = preferences_light.getUShort(buffer, false);
    sprintf(buffer, "brightness%d", currRoom);
    rooms[currRoom].brightness = preferences_light.getUShort(buffer, false);
    // unchangeable values
    rooms[currRoom].colorOn = strip.Color(rooms[currRoom].brightness, rooms[currRoom].brightness, rooms[currRoom].brightness);
    rooms[currRoom].colorOff = strip.Color(0, 0, 0);
    rooms[currRoom].status = lightOff;
    rooms[currRoom].goneTime = 0;
    rooms[currRoom].duration = rooms[currRoom].onTime + rooms[currRoom].offTime;
    strip.setPixelColor(currRoom, strip.Color(0, 0, 0));
  }
}

// Diese Funktion wird von der Browseranwendung direkt zu Beginn aufgeruen und
// gibt zurück, die Gesamtzeit eines Temperaturverlaufes, sowie die PWM-Werte der
// einzelnen Phasen.
/**
 * The function IPNbr() returns the local IP address as a string in C++.
 *
 * @return The function IPNbr() returns the local IP address of the device as a string.
 */
String IPNbr()
{
  String ipString = WiFi.localIP().toString().c_str();
  return ipString;
}

String GetParams()
{
  // sendet Informationen über die Räume an Browserseite
  uint8_t roomnbr = currRoom;
  String paramsString = String(rooms[roomnbr].onTime) + "/" + String(rooms[roomnbr].offTime) + "/" + String(rooms[roomnbr].brightness) + "/" + String(baseTime_house) + "/" + String(nutzung) + "/" + String(showMode) + "/";
  Serial.println("GetParams: " + paramsString);
  return paramsString;
}

String RaumLimits()
{
  String paramsString = String(raum_min) + "/" + String(raum_max) + "/" + String(raum_step) + "/" + String(currRoom);
  return paramsString;
}

String onTimeLimits()
{
  String paramsString = String(onTime_min) + "/" + String(onTime_max) + "/" + String(onTime_step) + "/" + String(rooms[currRoom].onTime);
  return paramsString;
}

String offTimeLimits()
{
  String paramsString = String(offTime_min) + "/" + String(offTime_max) + "/" + String(offTime_step) + "/" + String(rooms[currRoom].offTime);
  return paramsString;
}

String brightnessLimits()
{
  String paramsString = String(brightness_min) + "/" + String(brightness_max) + "/" + String(brightness_step) + "/" + String(rooms[currRoom].brightness);
  return paramsString;
}

String zeitfaktorLimits()
{
  String paramsString = String(zeitfaktor_min) + "/" + String(zeitfaktor_max) + "/" + String(zeitfaktor_step) + "/" + String(baseTime_house);
  return paramsString;
}

String nutzungLimits()
{
  String paramsString = String(nutzung_min) + "/" + String(nutzung_max) + "/" + String(nutzung_step) + "/" + String(nutzung);
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
  WiFi.macAddress(nativeMACAddress);
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
  ident = false;
  IPAddress ip = WiFi.localIP();
  // Print ESP32 Local IP Address
  Serial.print("IP: ");
  Serial.println(ip);

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

  MDNS.addService("http", "tcp", 80);

  strip.begin();
  // die preferences-Library wird gestartet

  if (preferences_light.begin(prefNameLight, false))
  {
    log_d("Preferences erfolgreich gestartet");
  }
  uint8_t setup_todo = preferences_light.getUChar("setup_done", 0xFF);
  if (setup_todo != setup_done)
  {
    randomSeed(nativeMACAddress[0] + nativeMACAddress[1] + nativeMACAddress[2] + nativeMACAddress[3] + nativeMACAddress[4] + nativeMACAddress[5]);
    for (uint8_t r = 0; r < strip.numPixels(); r++)
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
      rooms[r].colorOn = strip.Color(rooms[r].brightness, rooms[r].brightness, rooms[r].brightness);
      rooms[r].colorOff = strip.Color(0, 0, 0);
      rooms[r].status = lightOff;
      rooms[r].goneTime = 0;
      rooms[r].duration = rooms[r].onTime + rooms[r].offTime;
      strip.setPixelColor(r, strip.Color(0, 0, 0));
    }
    baseTime_house = zeitfaktor_curr; // delay between steps, in us, based on speed
    preferences_light.putUShort("baseTime", baseTime_house);
    nutzung = nutzung_curr; // delay between steps, in us, based on speed
    preferences_light.putUShort("nutzung", nutzung);
    currRoom = raum_curr;
    preferences_light.putUShort("currRoom", currRoom);
    showMode = house;
    preferences_light.putUShort("showMode", showMode);

    // setup_done auf "TRUE" setzen
    preferences_light.putUChar("setup_done", setup_done);
  }
  else
  {
    for (uint8_t r = 0; r < strip.numPixels(); r++)
    {
      // changeable values
      // onTime
      sprintf(buffer, "onTime%d", r);
      //  Serial.println("GET: " + String(buffer));
      rooms[r].onTime = preferences_light.getUShort(buffer, false);
      // offTime
      sprintf(buffer, "offTime%d", r);
      rooms[r].offTime = preferences_light.getUShort(buffer, false);
      // brightness
      sprintf(buffer, "brightness%d", r);
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
          strip.setPixelColor(r, rooms[r].colorOn);
        else
          strip.setPixelColor(r, rooms[r].colorOff);
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
      colorWipe(strip.Color(255, 0, 0), rooms[0].brightness); // Red
      break;
    case 1:
      colorWipe(strip.Color(0, 255, 0), rooms[0].brightness); // Green
      break;
    case 2:
      colorWipe(strip.Color(0, 0, 255), rooms[0].brightness); // Blue
      break;
    case 3:
      colorWipe(strip.Color(0, 0, 0, 255), rooms[0].brightness); // White RGBW
      break;
    case 4:
      // Send a theater pixel chase in...
      //      Serial.println("theaterChase White");
      theaterChase(strip.Color(127, 127, 127), rooms[0].brightness); // White
      break;
    case 5:
      //      Serial.println("theaterChase Red");
      theaterChase(strip.Color(127, 0, 0), rooms[0].brightness); // Red
      break;
    case 6:
      //      Serial.println("theaterChase Blue");
      theaterChase(strip.Color(0, 0, 127), rooms[0].brightness); // Blue
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
      theaterChaseRainbow(rooms[0].brightness);
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
        strip.setPixelColor(TestRoom, strip.Color(32, 32, 32));
      }
      else
      {
        LED_off();
        strip.setPixelColor(TestRoom, strip.Color(0, 0, 0));
        TestRoom++;
        if (TestRoom >= nutzung)
        {
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
