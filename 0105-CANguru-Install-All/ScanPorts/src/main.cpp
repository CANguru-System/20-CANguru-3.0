#include <Arduino.h>
#include <WiFi.h>
#include <EEPROM.h>
#include <Preferences.h>
#include "OWN_LED.h"
#include <nvs_flash.h>

const uint8_t maxstring = 96;
String command;
String subCommand;
String ssid;
String password;
Preferences preferences;

void netzwerkScan()
{
  // Zunächst Station Mode und Trennung von einem AccessPoint, falls dort eine Verbindung bestand
  //  WiFi.mode(WIFI_STA);
  //  WiFi.disconnect();
  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  if (n == 0)
  {
    printf("Keine Netzwerke gefunden!\r\n");
  }
  else
  {
    printf("&%dA\r\n", n);
    for (int i = 0; i < n; ++i)
    {
      // Drucke SSID and RSSI für jedes gefundene Netzwerk
      printf("%s (%d)\r\n", WiFi.SSID(i).c_str(), WiFi.RSSI(i));
      delay(10);
    }
  }
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
#ifdef ESP32C3
#define BLINKY_PIN 8
#else
#define BLINKY_PIN 2
#endif
  LED_begin(BLINKY_PIN);
  command.reserve(maxstring);
  subCommand.reserve(maxstring);
  ssid.reserve(maxstring);
  password.reserve(maxstring);

  if (preferences.begin("CANguru", false))
  {
    log_i("Preferences wurde erfolgreich gestartet");
  }
  LED_on();
}

void loop()
{
  // put your main code here, to run repeatedly:
  if (Serial.available() > 0)
  {
    command = Serial.readStringUntil('\n'); // read string until newline character
    subCommand = command.substring(0, 4);
    //    printf("%s\r\n", subCommand);
    if (command == "SCAN")
    {
      netzwerkScan();
      command = "";
    }
    if (subCommand == "SSID")
    {
      ssid = command.substring(4);
      Serial.println("&1B");
      Serial.println(ssid);
      preferences.putString("ssid", ssid);
      delay(10);
      command = "";
    }
    if (subCommand == "PASW")
    {
      password = command.substring(4, command.length());
      Serial.println("&1C");
      Serial.println(password);
      preferences.putString("password", password);
      delay(10);
      command = "";
    }
    if (command == "IPAD")
    {

      WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED)
      {
        delay(500);
      }
      IPAddress IP = WiFi.localIP();
      char ip[4]; // prepare a buffer for the data
      printf("&1D\r\n");
      for (uint8_t i = 0; i < 4; i++)
      {
        ip[i] = IP[i];
        if (i < 3)
          printf("%d.", IP[i]);
        else
          printf("%d\r\n", IP[i]);
      }
      preferences.putBytes("IP0", ip, 4);
      delay(10);
      preferences.end();
      command = "";
    }
  }
  delay(1000);
}
