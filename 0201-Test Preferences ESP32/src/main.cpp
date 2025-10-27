#include <Arduino.h>
#include <Preferences.h>

Preferences prefs;
// Beispielhafte Schlüssel
const char *keys[] = {"IP0", "ssid", "password"};

void setup()
{
  Serial.begin(115200);
  prefs.begin("CANguru", false); // Namespace "CANguru", nicht nur lesend
/*
  IPAddress ipadr(192, 168, 1, 42);
  uint32_t ipRaw = (uint32_t)ipadr; // IP in 32-Bit-Zahl umwandeln

  prefs.putUInt(keys[0], ipRaw); // Speichern
*/
  for (int8_t p = 0; p < sizeof(keys) / sizeof(keys[0]); p++)
  {
    if (p == 0)
    {
      uint32_t ip = prefs.getUInt(keys[p], 0); // Default = 0.0.0.0
      IPAddress IP(ip);
      Serial.printf("Key: %s => Value: %d.%d.%d.%d.\n\r", keys[p], IP[0], IP[1], IP[2], IP[3]);
      Serial.println(IP); // Gibt z. B. "192.168.1.42" aus
    }
    else
    {
      String value = prefs.getString(keys[p], "N/A");
      Serial.printf("Key: %s => Value: %s\n\r", keys[p], value.c_str());
    }
  }

  prefs.end();
}

void loop()
{
  // nichts zu tun
}
/*#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>

const char* ssid = "DEIN_SSID";
const char* password = "DEIN_PASSWORT";

AsyncWebServer server(80);
Preferences prefs;

String getPreferencesHTML() {
  prefs.begin("myApp", true); // read-only

  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Preferences</title></head><body>";
  html += "<h2>Gespeicherte Einträge</h2><table border='1'><tr><th>Schlüssel</th><th>Wert</th></tr>";

  String keyList = prefs.getString("keys", "");
  int start = 0;
  while (start < keyList.length()) {
    int comma = keyList.indexOf(',', start);
    if (comma == -1) comma = keyList.length();
    String key = keyList.substring(start, comma);
    String value = prefs.getString(key, "N/A");
    html += "<tr><td>" + key + "</td><td>" + value + "</td></tr>";
    start = comma + 1;
  }

  html += "</table></body></html>";
  prefs.end();
  return html;
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Verbinde mit WLAN...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nVerbunden! IP-Adresse: " + WiFi.localIP().toString());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = getPreferencesHTML();
    request->send(200, "text/html", html);
  });

  server.begin();
}

void loop() {
  // nichts zu tun
}
*/