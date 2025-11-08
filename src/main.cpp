#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <esp_wifi.h>
#include <esp_wpa2.h>

#include "include/config.h"
#include "include/crypto.h"
#include "include/member.h"
#include "include/network.h"
#include "include/secrets.h"
#include "include/sniffer.h"
#include "include/time_utils.h"
#include "mbedtls/md.h"
#include "time.h"

HTTPClient http;
WiFiClientSecure client;

char dateStr[11];

JsonDocument memberData;
std::map<String, String> memberMap;
std::map<String, String> hmacMap;

esp_timer_handle_t channelHop_timer;

uint8_t newMACAddress[] = MAC_ADDRESS;

void setup() {
  Serial.begin(115200);

  client.setInsecure();
  client.setHandshakeTimeout(30);

  WiFi.mode(WIFI_STA);

  Serial.print("DEFAULT MAC Address: ");
  readMacAddress();

  esp_err_t err = esp_wifi_set_mac(WIFI_IF_STA, &newMACAddress[0]);
  if (err == ESP_OK) {
    Serial.println("Mac Address Changed");
  }

  Serial.print("Spoofed MAC Address: ");
  readMacAddress();

  if (is_enterprise) {
    init_peap();
  }

  fetchMemberData();

  struct tm currentTime = getTimeInfo();
  strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", &currentTime);
  Serial.println("\nDate: " + String(dateStr));

  createHMACMap();

  delay(10);

  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous_rx_cb(&sniffer_callback);
  esp_wifi_set_promiscuous(true);

  const esp_timer_create_args_t timer_args = {.callback = &channelHop,
                                              .arg = NULL,
                                              .dispatch_method = ESP_TIMER_TASK,
                                              .name = "channel_hop"};
  esp_timer_create(&timer_args, &channelHop_timer);
  esp_timer_start_periodic(channelHop_timer, 500 * 1000);
}

void loop() {
  delay(90000);
  sendToServer();
}
