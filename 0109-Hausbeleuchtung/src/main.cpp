#include <Arduino.h>

#include <Adafruit_NeoPixel.h>
#include "preferences.h"

// Import required libraries
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "LittleFS.h"
#include "CANguruDefs.h"

Preferences preferences_light;
Preferences preferences_CANguru;

// Replace with your network credentials

const gpio_num_t LED_PIN_HOUSELIGHT0 = GPIO_NUM_0;
const gpio_num_t LED_PIN_HOUSELIGHT1 = GPIO_NUM_1;
const gpio_num_t LED_PIN_HOUSELIGHT2 = GPIO_NUM_2;
const gpio_num_t LED_PIN_HOUSELIGHT3 = GPIO_NUM_3;
const gpio_num_t LED_PIN_HOUSELIGHT4 = GPIO_NUM_4;

#define LED_COUNT 8

const uint8_t max_pins = 5;

uint8_t curr_pin = LED_PIN_HOUSELIGHT2;

gpio_num_t pins[max_pins] = {LED_PIN_HOUSELIGHT0, LED_PIN_HOUSELIGHT1, LED_PIN_HOUSELIGHT2, LED_PIN_HOUSELIGHT3, LED_PIN_HOUSELIGHT4};

Adafruit_NeoPixel houselight(LED_COUNT, LED_PIN_HOUSELIGHT2, NEO_RGB + NEO_KHZ800);

#define prefNameLight "HOUSELIGHT"
#define setup_done 0x47
#define setup_NOTdone 0xFF

const char del = '!';

unsigned long lastTime_house; // timestamp in us of when the last step was taken
unsigned long baseTime_house; // delay between steps, in us, based on speed

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
roomType rooms[LED_COUNT][max_pins];

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Replaces placeholder with LED state value
String processor(const String &var)
{
  uint8_t room;
  String roomNbr;
  char buffer[12];
  char aspect;
  uint16_t value16;
  uint32_t value32;
  aspect = var.charAt(0);
  roomNbr = var.substring(1, 2);
  Serial.println("roomNbr: " + roomNbr);
  room = roomNbr.toInt();
  switch (aspect)
  {
  case 'n': // onTime
    value16 = rooms[room][curr_pin].onTime;
    sprintf(buffer, "%d", value16);
    break;
  case 'f': // offTime
    value16 = rooms[room][curr_pin].offTime;
    sprintf(buffer, "%d", value16);
    break;
  case 'b': // brightness
    value16 = rooms[room][curr_pin].brightness;
    sprintf(buffer, "%d", value16);
    break;
  case 't': // Basiszeit
    value32 = baseTime_house;
    sprintf(buffer, "%d", value32);
    break;

  default:
    break;
  }
  return buffer;

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
  uint8_t v0;
  uint8_t v1;
  uint8_t v2;
  uint8_t v3;
  uint8_t v4;
  uint16_t v5;
  if (urlLine.indexOf("/onTime") >= 0)
  {
    Serial.println("Message: " + String(urlLine));
  }
  if (urlLine.indexOf("/offTime") >= 0)
  {
    Serial.println("Message: " + String(urlLine));
  }
  if (urlLine.indexOf("/SetParams") >= 0)
  {
    Serial.println("Message: " + String(urlLine));
    for (uint8_t p = 0; p < 6; p++)
    {
      // beginne mit dem 2. Parameter (ohne /PARAM)
      val = getStringPartByNr(urlLine, '/', p + 2);
      switch (p)
      {
      case 0:
        v0 = (uint8_t)val.toInt();
        //      EEPROM.writeByte(eepromAdrParameter0, preheatPWM);
        break;
      case 1:
        v1 = (uint8_t)val.toInt();
        //      EEPROM.writeByte(eepromAdrParameter1, soakPWM);
        break;
      case 2:
        v2 = (uint8_t)val.toInt();
        //      EEPROM.writeByte(eepromAdrParameter2, reflowPWM);
        break;
      case 3:
        v3 = (uint8_t)val.toInt();
        //      EEPROM.writeByte(eepromAdrParameter6, preTimeFactor);
        break;
      case 4:
        v4 = (uint8_t)val.toInt();
        //      EEPROM.writeByte(eepromAdrParameter6, preTimeFactor);
        break;
      case 5:
        v5 = (uint16_t)val.toInt();
        //      EEPROM.writeByte(eepromAdrParameter6, preTimeFactor);
        break;
      }
      //    EEPROM.commit();
    }
    Serial.println(v0);
    Serial.println(v1);
    Serial.println(v2);
    Serial.println(v3);
    Serial.println(v4);
    Serial.println(v5);
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
  uint8_t roomnbr = 0;
  String paramsString = String(rooms[roomnbr][curr_pin].onTime) + "/" + String(rooms[roomnbr][curr_pin].offTime) + "/" + String(rooms[roomnbr][curr_pin].brightness) + "/" + String(baseTime_house);
  return paramsString;
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
  WiFi.begin(ssidCh, passwordCh);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  preferences_CANguru.end();

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/index.html", String(), false, processor); });

  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/style.css", "text/css"); });

  server.on("/IPNbr", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", IPNbr().c_str()); });

  server.on("/GetParams", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", GetParams().c_str()); });

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
      rooms[r][curr_pin].onTime = random(10) + r;
      sprintf(buffer, "onTime%d", r);
      preferences_light.putUShort(buffer, rooms[r][curr_pin].onTime);
      // offTime
      rooms[r][curr_pin].offTime = random(5) + 2 * r;
      sprintf(buffer, "offTime%d", r);
      preferences_light.putUShort(buffer, rooms[r][curr_pin].offTime);
      // brightness
      rooms[r][curr_pin].brightness = 16;
      sprintf(buffer, "brightness%d", r);
      preferences_light.putUShort(buffer, rooms[r][curr_pin].brightness);
      // unchangeable values
      rooms[r][curr_pin].colorOn = houselight.Color(rooms[r][curr_pin].brightness, rooms[r][curr_pin].brightness, rooms[r][curr_pin].brightness);
      rooms[r][curr_pin].colorOff = houselight.Color(0, 0, 0);
      rooms[r][curr_pin].status = lightOff;
      rooms[r][curr_pin].goneTime = 0;
      rooms[r][curr_pin].duration = rooms[r][curr_pin].onTime + rooms[r][curr_pin].offTime;
      houselight.setPixelColor(r, houselight.Color(0, 0, 0));
    }
    baseTime_house = 1000; // delay between steps, in us, based on speed
    preferences_light.putUShort("baseTime", baseTime_house);
    // setup_done auf "TRUE" setzen
    preferences_light.putUChar("setup_done", setup_done);
  }
  else
  {
    for (uint8_t r = 0; r < houselight.numPixels(); r++)
    {
      // changeable values
      sprintf(buffer, "onTime%d", r);
      rooms[r][curr_pin].onTime = preferences_light.getUShort(buffer, false);
      sprintf(buffer, "offTime%d", r);
      rooms[r][curr_pin].offTime = preferences_light.getUShort(buffer, false);
      sprintf(buffer, "brightness%d", r);
      rooms[r][curr_pin].brightness = preferences_light.getUShort(buffer, false);
      // unchangeable values
      rooms[r][curr_pin].colorOn = houselight.Color(rooms[r][curr_pin].brightness, rooms[r][curr_pin].brightness, rooms[r][curr_pin].brightness);
      rooms[r][curr_pin].colorOff = houselight.Color(0, 0, 0);
      rooms[r][curr_pin].status = lightOff;
      rooms[r][curr_pin].goneTime = 0;
      rooms[r][curr_pin].duration = rooms[r][curr_pin].onTime + rooms[r][curr_pin].offTime;
      houselight.setPixelColor(r, houselight.Color(0, 0, 0));
    }
    baseTime_house = preferences_light.getUShort("baseTime", baseTime_house);
  }
  houselight.show();
}

void loop()
{
  // Hausbeleuchtung
  if (millis() - lastTime_house >= baseTime_house)
  {
    // get the timeStamp of when you stepped:
    lastTime_house = millis();
    for (uint8_t r = 0; r < houselight.numPixels(); r++)
    {
      rooms[r][curr_pin].goneTime++;
      if (rooms[r][curr_pin].goneTime > rooms[r][curr_pin].duration)
      {
        rooms[r][curr_pin].goneTime = 0;
        rooms[r][curr_pin].status = lightOff;
      }
      if (rooms[r][curr_pin].goneTime > rooms[r][curr_pin].offTime)
      {
        rooms[r][curr_pin].status = lightOn;
      }
      if (rooms[r][curr_pin].status == lightOn)
        houselight.setPixelColor(r, rooms[r][curr_pin].colorOn);
      else
        houselight.setPixelColor(r, rooms[r][curr_pin].colorOff);
      houselight.show();
    }
  }
}
