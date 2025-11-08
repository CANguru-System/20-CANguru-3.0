#include <Arduino.h>

#include <LittleFS.h>

void setup() {
  Serial.begin(115200);

  // Versuche zu mounten
  if (!LittleFS.begin()) {
    Serial.println("Fehler beim Mounten. Versuche Formatierung...");
    LittleFS.format(); // Achtung: löscht alle Dateien!
    
    if (LittleFS.begin()) {
      Serial.println("LittleFS erfolgreich nach Formatierung gemountet.");
    } else {
      Serial.println("Formatierung fehlgeschlagen.");
    }
  } else {
    Serial.println("LittleFS erfolgreich gemountet.");
  }
}

void loop() {
  // put your main code here, to run repeatedly:
}
