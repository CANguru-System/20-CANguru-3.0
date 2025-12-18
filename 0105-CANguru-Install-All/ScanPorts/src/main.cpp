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
String host;
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
  uint8_t m = 0;
  uint8_t n = WiFi.scanNetworks();
  if (n == 0)
  {
    printf("Keine Netzwerke gefunden!\r\n");
  }
  else
  {
    for (int i = 0; i < n; ++i)
    {
      // Drucke SSID and RSSI für jedes gefundene Netzwerk, außer
      if (!WiFi.SSID(i).startsWith("CNgrSLV"))
        m++;
    }

    printf("&%dA\r\n", m);
    for (int j = 0; j < n; ++j)
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
      if (!WiFi.SSID(j).startsWith("CNgrSLV"))
        printf("%s (%d)\r\n", WiFi.SSID(j).c_str(), WiFi.RSSI(j));
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
  /*  command.reserve(maxstring);
    subCommand.reserve(maxstring);
    ssid.reserve(maxstring);
    password.reserve(maxstring);
  */
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
  char sourcebuffer[maxstring]; // Enough to hold 3 digits and a null terminator
  char destbuffer[maxstring];   // Enough to hold 3 digits and a null terminator
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
      ssid = command.substring(4, command.length());
      // Speichern des SSID in den Preferences
      preferences.putString("ssid", ssid);
      // Antwort an InstallGUI
      Serial.println("&1B");
      Serial.println(ssid);
      delay(10);
      command = "";
    }
    if (subCommand == "PASW")
    {
      password = command.substring(4, command.length());
      preferences.putString("password", password);
      // Antwort an InstallGUI
      Serial.println("&1C");
      Serial.println(password);
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

        //        connectionStatusMessage(status);
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
      uint32_t ipRaw = (uint32_t)IP;     // IP in 32-Bit-Zahl umwandeln
      preferences.putUInt("IP0", ipRaw); // Speichern
      // Antwort an InstallGUI
      Serial.println("&1D");
      Serial.println(IP);
      delay(10);
      command = "";
    }
    if (subCommand == "HOST")
    {
      host = command.substring(4, command.length());
      preferences.putString("HOST", host);
      Serial.println("&1E");
      Serial.println(host);
      delay(10);
      command = "";
    }
  }
  delay(500);
}
