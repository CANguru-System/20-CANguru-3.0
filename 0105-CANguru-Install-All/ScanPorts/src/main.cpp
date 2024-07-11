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
const uint8_t wifItrialsMax = 10;
uint8_t wifItrials;
IPAddress IP;

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
  /*
  RSSI Value Range	WiFi Signal Strength
  RSSI > -30 dBm	 Amazing
  RSSI < – 55 dBm	 Very good signal
  RSSI < – 67 dBm	 Fairly Good
  RSSI < – 70 dBm	 Okay
  RSSI < – 80 dBm	 Not good
  RSSI < – 90 dBm	 Extremely weak signal (unusable)
  */
      // Drucke SSID and RSSI für jedes gefundene Netzwerk
      printf("%s (%d)\r\n", WiFi.SSID(i).c_str(), WiFi.RSSI(i));
      delay(10);
    }
  }
}

#ifdef ESP32C3
void stopStepper()
{
  uint8_t A_plus = GPIO_NUM_10;
  uint8_t A_minus = GPIO_NUM_7;
  uint8_t B_plus = GPIO_NUM_6;
  uint8_t B_minus = GPIO_NUM_5;
  pinMode(A_plus, OUTPUT);
  pinMode(A_minus, OUTPUT);
  pinMode(B_plus, OUTPUT);
  pinMode(B_minus, OUTPUT);
  digitalWrite(A_plus, LOW);
  digitalWrite(A_minus, LOW);
  digitalWrite(B_plus, LOW);
  digitalWrite(B_minus, LOW);
}
#endif

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
#ifdef ESP32C3
  stopStepper();
#endif
}

void connectionStatusMessage(wl_status_t st)
{
  switch (st)
  {
  case WL_NO_SHIELD:
    log_d("Status: %d - WL_NO_SHIELD", st);
    break;
  case WL_IDLE_STATUS:
    log_d("Status: %d - WL_IDLE_STATUS", st);
    break;
  case WL_NO_SSID_AVAIL:
    log_d("Status: %d - WL_NO_SSID_AVAIL", st);
    break;
  case WL_SCAN_COMPLETED:
    log_d("Status: %d - WL_SCAN_COMPLETED", st);
    break;
  case WL_CONNECTED:
    log_d("Status: %d - WL_CONNECTED", st);
    break;
  case WL_CONNECT_FAILED:
    log_d("Status: %d - WL_CONNECT_FAILED", st);
    break;
  case WL_CONNECTION_LOST:
    log_d("Status: %d - WL_CONNECTION_LOST", st);
    break;
  case WL_DISCONNECTED:
    log_d("Status: %d - WL_DISCONNECTED", st);
    break;

  default:
    log_d("Status: %d - WL_OK", st);
    break;
  }
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
      wifItrials = wifItrialsMax;
      WiFi.mode(WIFI_STA);
      wl_status_t status = WiFi.begin(ssid, password);
      while ((status != WL_CONNECTED && wifItrials > 0))
      {
        status = (wl_status_t)WiFi.waitForConnectResult(2000);

        connectionStatusMessage(status);
        wifItrials--;
      }
      if (wifItrials == 0)
      {
        IP[0] = 0;
        IP[1] = 0;
        IP[2] = 0;
        IP[3] = 0;
      }
      else
      {
        IP = WiFi.localIP();
      }
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
