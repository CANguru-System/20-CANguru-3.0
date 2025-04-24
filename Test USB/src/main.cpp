#include <Arduino.h>

void setup() {
  Serial.begin(115200); // Initialize serial communication at 115200 baud rate
}

void loop() {
  Serial.println("Hello from ESP32!"); // Send data to the PC
  delay(1000); // Wait for a second
}
