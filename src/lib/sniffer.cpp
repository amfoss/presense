#include <WiFi.h>
#include <esp_wifi.h>
#include <vector>
#include <string>
#include "../include/sniffer.h"
#include "../include/config.h"

void showMetadata(SnifferPacket *snifferPacket) {
  unsigned int frameControl = ((unsigned int)snifferPacket->data[1] << 8) + snifferPacket->data[0];

  uint8_t version      = (frameControl & 0b0000000000000011) >> 0;
  uint8_t frameType    = (frameControl & 0b0000000000001100) >> 2;
  uint8_t frameSubType = (frameControl & 0b0000000011110000) >> 4;
  uint8_t toDS         = (frameControl & 0b0000000100000000) >> 8;
  uint8_t fromDS       = (frameControl & 0b0000001000000000) >> 9;

  // Only look for probe request packets
  if (frameType != TYPE_MANAGEMENT || frameSubType != SUBTYPE_PROBE_REQUEST)
      return;

  char addr[18] = "00:00:00:00:00:00";
  getMAC(addr, snifferPacket->data, 10);
  
  Serial.print("MAC: ");
  Serial.print(addr);
  
  std::string addrStr(addr);
  if (std::find(foundMacAddresses.begin(), foundMacAddresses.end(), addrStr) == foundMacAddresses.end()) {
    foundMacAddresses.push_back(addrStr);
  }

  uint8_t SSID_length = snifferPacket->data[25];
  Serial.println();
}

void sniffer_callback(void* buf, wifi_promiscuous_pkt_type_t type) {
  if (type == WIFI_PKT_MGMT) {
    SnifferPacket *snifferPacket = (SnifferPacket*) buf;
    showMetadata(snifferPacket);
  }
}

void printDataSpan(uint16_t start, uint16_t size, uint8_t* data) {
  for(uint16_t i = start; i < DATA_LENGTH && i < start + size; i++) {
    Serial.write(data[i]);
  }
}

void getMAC(char *addr, uint8_t* data, uint16_t offset) {
  sprintf(addr, "%02x:%02x:%02x:%02x:%02x:%02x", data[offset+0], data[offset+1], data[offset+2], data[offset+3], data[offset+4], data[offset+5]);
}

#define CHANNEL_HOP_INTERVAL_MS   1000

void channelHop(void* arg) {
    uint8_t primary_channel;
    wifi_second_chan_t second_channel;
    esp_err_t err = esp_wifi_get_channel(&primary_channel, &second_channel);
    if (err == ESP_OK) {
        uint8_t new_channel = (primary_channel % 13) + 1;
        esp_wifi_set_channel(new_channel, WIFI_SECOND_CHAN_NONE);
    } else {
        Serial.println("Failed to get current Wi-Fi channel.");
    }
}