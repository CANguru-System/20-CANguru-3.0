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
// const char *ssid = "FRITZ!Box 6591 Cable RC_EXT";
// const char *password = "Ko-Cc3!dRe!";

#define LED_PIN_HOUSELIGHT 2
#define LED_PIN_RUNNINGLIGHT 3
#define LED_COUNT 8

Adafruit_NeoPixel houselight(LED_COUNT, LED_PIN_HOUSELIGHT, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel runninglight(LED_COUNT, LED_PIN_RUNNINGLIGHT, NEO_RGB + NEO_KHZ800);

#define prefNameLight "HOUSELIGHT"
#define setup_done 0x47
#define setup_NOTdone 0xFF

const char del = '!';

unsigned long lastTime_house;     // timestamp in us of when the last step was taken
unsigned long baseTime_house;     // delay between steps, in us, based on speed
unsigned long lastTime_run;       // timestamp in us of when the last step was taken
unsigned long baseTime_run = 200; // delay between steps, in us, based on speed

enum statusType
{
  lightOn,
  lightOff
};

enum runstatusType
{
  run,
  blink0,
  blink1,
  blink2,
  blink3,
  blink4
};
runstatusType runstatus = run;

struct runlightType
{
  statusType status;
};
runlightType runLEDs[LED_COUNT];

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
roomType rooms[LED_COUNT];

String command;
String load;
int8_t runled = 0;

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
    value16 = rooms[room].onTime;
    sprintf(buffer, "%d", value16);
    break;
  case 'f': // offTime
    value16 = rooms[room].offTime;
    sprintf(buffer, "%d", value16);
    break;
  case 'b': // brightness
    value16 = rooms[room].brightness;
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

void light(statusType s)
{
  for (uint8_t r = 0; r < runninglight.numPixels(); r++)
  {
    // Set the first LED to red
    runLEDs[r].status = s;
  }
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
  if (urlLine.indexOf("/PARAM") >= 0)
  {
    Serial.println("Message: " + String(urlLine));
    for (uint8_t p = 0; p < 4; p++)
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
      }
      //    EEPROM.commit();
    }
    Serial.println(v0);
    Serial.println(v1);
    Serial.println(v2);
    Serial.println(v3);
  }
}
// Diese Funktion wird von der Browseranwendung direkt zu Beginn aufgeruen und
// gibt zurück, die Gesamtzeit eines Temperaturverlaufes, sowie die PWM-Werte der
// einzelnen Phasen.
String IPNbr()
{
  String ipString = WiFi.localIP().toString().c_str();
  Serial.println(ipString);
  return ipString;
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

  // Route to set GPIO to HIGH
  server.on("/save", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/index.html", String(), false, processor); });

  // Start server
  server.onNotFound(onRequest);
  server.begin();

  houselight.begin();
  runninglight.begin();
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
      preferences_light.putUShort(buffer, rooms[r].onTime);
      // offTime
      rooms[r].offTime = random(5) + 2 * r;
      sprintf(buffer, "offTime%d", r);
      preferences_light.putUShort(buffer, rooms[r].offTime);
      // brightness
      rooms[r].brightness = 16;
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
  }
  houselight.show();

  light(lightOff);
  for (int8_t r = 0; r < runninglight.numPixels(); r++)
    if (runLEDs[r].status == lightOn)
      runninglight.setPixelColor(r, runninglight.Color(16, 16, 16)); // white color
    else
      runninglight.setPixelColor(r, runninglight.Color(0, 0, 0)); // white color
  runninglight.show();

  lastTime_house = millis();
  lastTime_run = millis();
}

void loop()
{
  /*
    if (Serial.available() > 0)
    {
      // read the incoming byte:
      char inByte = Serial.read();
      command += inByte;
      if (inByte == '$')
      {
        //
        // Mit dem Kommando DECX werden die Daten des Raumes X (zwischen 0 und 7) zu einem String ontime!offtime!brightness!basetime
        // zusammengefügt und übermittelt
        if (command.substring(0, 3) == "DEC")
        {
          Serial.println("&A");
          delay(50);
          uint8_t d = command.charAt(3) - 0x30;
          load = String(rooms[d].onTime) + del + String(rooms[d].offTime) + del + String(rooms[d].brightness) + del + baseTime_house;
          Serial.println(load);
        }
        if (command.substring(0, 3) == "RES")
        {
          // setup_done auf "TRUE" setzen
          preferences_light.putUChar("setup_done", setup_NOTdone);
          ESP.restart();
        }
        // Alle gefundenen USB-Ports werden mit "USB0" aufgerufen.
        // Nur die richtigen Hausbeleuchtungen antworten mit "USB"
        if (command.substring(0, 4) == "USB0")
        {
          Serial.println("&B");
          delay(50);
          Serial.println("USB");
        }
        // Ein neuer geänderter Datensatz wird mit der Länge XXX übermittelt. Der Trenner ist ein "!"
        // L!index!ontime!offtime!brightness!basetime
        // L!3!27!39!32!1500$
        // cnt = 12;
        if (command.substring(0, 1) == "L")
        {
          int startIndex = 0;
          int8_t index;
          String indx;
          uint8_t substr = 0;
          char buffer[25]; // Enough to hold 3 digits and a null terminator
          Serial.println("&C");
          delay(50);
          for (int i = 0; i < command.length(); i++)
          {
            if (command[i] == del)
            {
              String substring = command.substring(startIndex, i);
              switch (substr)
              {
              case 0:
                // "L"
                substr++;
                break;
              case 1:
                // index
                index = substring.toInt();
                substr++;
                break;
              // changeable values
              case 2:
                // lightOn
                sprintf(buffer, "onTime%d", index);
                rooms[index].onTime = substring.toInt();
                preferences_light.putUShort(buffer, rooms[index].onTime);
                substr++;
                break;
              case 3:
                // lightOff
                sprintf(buffer, "offTime%d", index);
                rooms[index].offTime = substring.toInt();
                preferences_light.putUShort(buffer, rooms[index].offTime);
                substr++;
                break;
              case 4:
                // brightness
                sprintf(buffer, "brightness%d", index);
                rooms[index].brightness = substring.toInt();
                preferences_light.putUShort(buffer, rooms[index].brightness);
                substr++;
                break;
              default:
                break;
              }
              startIndex = i + 1;
            }
          }
          // baseTime
          baseTime_house = command.substring(startIndex).toInt();
          preferences_light.putUShort("baseTime", baseTime_house);
          // unchangeable values
          rooms[index].colorOn = houselight.Color(rooms[index].brightness, rooms[index].brightness, rooms[index].brightness);
          rooms[index].colorOff = houselight.Color(0, 0, 0);
          rooms[index].status = lightOff;
          rooms[index].goneTime = 0;
          rooms[index].duration = rooms[index].onTime + rooms[index].offTime;
          houselight.setPixelColor(index, houselight.Color(0, 0, 0));
        }
        command = "";
      }
    }
      */
  // Hausbeleuchtung
  if (millis() - lastTime_house >= baseTime_house)
  {
    // get the timeStamp of when you stepped:
    lastTime_house = millis();
    for (uint8_t r = 0; r < houselight.numPixels(); r++)
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
  // Lauflicht
  if (millis() - lastTime_run >= baseTime_run)
  {
    // get the timeStamp of when you stepped:
    lastTime_run = millis();
    switch (runstatus)
    {
    case run:
      //
      runLEDs[runled].status = lightOn;
      runled++;
      if (runled == runninglight.numPixels())
      {
        runled = 0;
        runstatus = blink0;
      }
      break;

    case blink0:
      light(lightOff);
      runstatus = blink1;
      break;
    case blink1:
      light(lightOn);
      runstatus = blink2;
      break;
    case blink2:
      light(lightOff);
      runstatus = blink3;
      break;
    case blink3:
      light(lightOn);
      runstatus = blink4;
      break;
    case blink4:
      light(lightOff);
      runstatus = run;
      break;

    default:
      break;
    }
    for (int8_t r = 0; r < runninglight.numPixels(); r++)
      if (runLEDs[r].status == lightOn)
        runninglight.setPixelColor(r, runninglight.Color(16, 16, 16)); // white color
      else
        runninglight.setPixelColor(r, runninglight.Color(0, 0, 0)); // white color
    runninglight.show();
  }
}
