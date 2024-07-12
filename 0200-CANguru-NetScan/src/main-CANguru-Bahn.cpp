#include <Arduino.h>

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"

#define MAXIMUM_AP 20
String ssidSLV = "CNgrSLV";

  wifi_scan_config_t scan_config = {
      .ssid = 0,
      .bssid = 0,
      .channel = 0,
      .show_hidden = true};

void setup()
{
  Serial.begin(115200);
  delay(500);
  Serial.println("NetworkScan");

  wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&wifi_config));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_start());
}

void loop()
{
  ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));

  wifi_ap_record_t wifi_records[MAXIMUM_AP];

  uint16_t max_records = MAXIMUM_AP;
  ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&max_records, wifi_records));
  int j = 1;
  for (int i = 0; i < max_records; i++)
  {
    String SSID((char *)wifi_records[i].ssid);
    if (SSID.indexOf(ssidSLV) == 0)
    {
      printf("%.2d - %s\r\n", j, wifi_records[i].ssid);
      j++;
    }
  }
  printf("\r\n\r\n");
  delay(5000);
}