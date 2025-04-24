#include <Arduino.h>

#include <Adafruit_NeoPixel.h>
#include "Preferences.h"

#define LED_PIN 2
#define LED_COUNT 8

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ800);
Preferences preferences;
#define prefName "HOUSELIGHT"
#define setup_done 0x47

uint8_t cnt;

unsigned long lastTime; // timestamp in us of when the last step was taken
unsigned long baseTime; // delay between steps, in us, based on speed

enum statusType
{
  lightOn,
  lightoff
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
roomType rooms[LED_COUNT];

int16_t random(int MaxZahl)
{
  float zufall;
  zufall = (float)rand() / RAND_MAX * MaxZahl + 1;
  return ((int16_t)zufall);
}

void setup()
{
  char buffer[25]; // Enough to hold 3 digits and a null terminator
  Serial.begin(115200);
  strip.begin();
  // die preferences-Library wird gestartet

  if (preferences.begin(prefName, false))
  {
    log_d("Preferences erfolgreich gestartet");
  }
  uint8_t setup_todo = preferences.getUChar("setup_done", 0xFF);
  if (setup_todo != setup_done)
  {
    for (uint8_t r = 0; r < strip.numPixels(); r++)
    {
      // changeable values
      rooms[r].onTime = random(10) + r;
      sprintf(buffer, "onTime%d", r);
      preferences.putUShort(buffer, rooms[r].onTime);
      rooms[r].offTime = random(5) + 2 * r;
      sprintf(buffer, "offTime%d", r);
      preferences.putUShort(buffer, rooms[r].offTime);
      rooms[r].brightness = 16;
      sprintf(buffer, "brightness%d", r);
      preferences.putUShort(buffer, rooms[r].brightness);
      // unchangeable values
      rooms[r].colorOn = strip.Color(rooms[r].brightness, rooms[r].brightness, rooms[r].brightness);
      rooms[r].colorOff = strip.Color(0, 0, 0);
      rooms[r].status = lightoff;
      rooms[r].goneTime = 0;
      rooms[r].duration = rooms[r].onTime + rooms[r].offTime;
      strip.setPixelColor(r, strip.Color(0, 0, 0));
    }
    baseTime = 1000; // delay between steps, in us, based on speed
    preferences.putUShort("baseTime", baseTime);
    // setup_done auf "TRUE" setzen
    preferences.putUChar("setup_done", setup_done);
  }
  else
  {
    for (uint8_t r = 0; r < strip.numPixels(); r++)
    {
      // changeable values
      sprintf(buffer, "onTime%d", r);
      rooms[r].onTime = preferences.getUShort(buffer, false);
      sprintf(buffer, "offTime%d", r);
      rooms[r].offTime = preferences.getUShort(buffer, false);
      sprintf(buffer, "brightness%d", r);
      rooms[r].brightness = preferences.getUShort(buffer, false);
      // unchangeable values
      rooms[r].colorOn = strip.Color(rooms[r].brightness, rooms[r].brightness, rooms[r].brightness);
      rooms[r].colorOff = strip.Color(0, 0, 0);
      rooms[r].status = lightoff;
      rooms[r].goneTime = 0;
      rooms[r].duration = rooms[r].onTime + rooms[r].offTime;
      strip.setPixelColor(r, strip.Color(0, 0, 0));
    }
    baseTime = preferences.getUShort("baseTime", baseTime);
  }
  strip.show();

  lastTime = millis();
  cnt = 4;
}

void loop()
{
  String command;
  String subCommand1;
  String subCommand3;
  String load;
  
  if (Serial.available() == cnt)
  {
    command = Serial.readString(); //('\n'); // read string until newline character
    subCommand1 = command.substring(0, 1);
    subCommand3 = command.substring(0, 3);
    //
    // Mit dem Kommando DECX werden die Daten des Raumes X (zwischen 0 und 7) zu einem String ontime!offtime!brightness!basetime
    // zusammengefügt und übermittelt
    if (subCommand3 == "DEC")
    {
      uint8_t d = command.charAt(3) - 0x30;
      load = String(rooms[d].onTime) + "!" + String(rooms[d].offTime) + "!" + String(rooms[d].brightness) + "!" + baseTime;
      Serial.println("&1A");
      Serial.println(load);
      subCommand3.clear();
      command.clear();
      exit;
    }
    // Alle gefundenen USB-Ports werden mit "USB0" aufgerufen.
    // Nur die richtigen Hausbeleuchtungen antworten mit "USB"
    if (command == "USB0")
    {
      Serial.println("&1B");
      Serial.println("USB");
      command.clear();
      exit;
    }
    // Mit TXXX wird die Länge (XXX) des folgenden Kommandos übermittelt
    if (subCommand1 == "T")
    {
      String sCnt = command.substring(1, 4);
      cnt = sCnt.toInt();
      Serial.println("&1C");
      Serial.println(cnt);
      subCommand1.clear();
      command.clear();
      exit;
    }
    // Ein neuer geänderter Datensatz wird mit der Länge XXX übermittelt. Der Trenner ist ein "!"
    // L!index!ontime!offtime!brightness!basetime
    // L!3!27!39!32!1500
    // cnt = 12;
    if (subCommand1 == "L")
    {
      int startIndex = 0;
      char del = '!';
      int8_t index;
      String indx;
      uint8_t substr = 0;
      char buffer[25]; // Enough to hold 3 digits and a null terminator
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
            preferences.putUShort(buffer, rooms[index].onTime);
            substr++;
            break;
          case 3:
            // lightOn
            sprintf(buffer, "offTime%d", index);
            rooms[index].offTime = substring.toInt();
            preferences.putUShort(buffer, rooms[index].offTime);
            substr++;
            break;
          case 4:
            // lightOn
            sprintf(buffer, "brightness%d", index);
            rooms[index].brightness = substring.toInt();
            preferences.putUShort(buffer, rooms[index].brightness);
            substr++;
            break;
          default:
            break;
          }
          startIndex = i + 1;
        }
      }
      baseTime = command.substring(startIndex).toInt();
      preferences.putUShort("baseTime", baseTime);
      // unchangeable values
      rooms[index].colorOn = strip.Color(rooms[index].brightness, rooms[index].brightness, rooms[index].brightness);
      rooms[index].colorOff = strip.Color(0, 0, 0);
      rooms[index].status = lightoff;
      rooms[index].goneTime = 0;
      rooms[index].duration = rooms[index].onTime + rooms[index].offTime;
      strip.setPixelColor(index, strip.Color(0, 0, 0));
      Serial.println("&1D");
      cnt = 4;
      subCommand1.clear();
      command.clear();
      exit;
    }
  }
/*  if (millis() - lastTime >= baseTime)
  {
    // get the timeStamp of when you stepped:
    lastTime = millis();
    for (uint8_t r = 0; r < strip.numPixels(); r++)
    {
      rooms[r].goneTime++;
      if (rooms[r].goneTime > rooms[r].duration)
      {
        rooms[r].goneTime = 0;
        rooms[r].status = lightoff;
      }
      if (rooms[r].goneTime > rooms[r].offTime)
      {
        rooms[r].status = lightOn;
      }
      if (rooms[r].status == lightOn)
        strip.setPixelColor(r, rooms[r].colorOn);
      else
        strip.setPixelColor(r, rooms[r].colorOff);
      strip.show();
    }
  }*/
}