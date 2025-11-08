#ifndef SNIFFER_H
#define SNIFFER_H

#include <WiFi.h>
#include <esp_wifi.h>
#include <vector>
#include <string>

#define DATA_LENGTH           112
#define TYPE_MANAGEMENT       0x00
#define TYPE_CONTROL          0x01
#define TYPE_DATA             0x02
#define SUBTYPE_PROBE_REQUEST 0x04
#define CHANNEL_HOP_INTERVAL_MS   1000

struct SnifferPacket {
    wifi_pkt_rx_ctrl_t rx_ctrl;
    uint8_t data[DATA_LENGTH];
    uint16_t cnt;
    uint16_t len;
};

extern std::vector<std::string> foundMacAddresses;

void showMetadata(SnifferPacket *snifferPacket);
void sniffer_callback(void* buf, wifi_promiscuous_pkt_type_t type);
void printDataSpan(uint16_t start, uint16_t size, uint8_t* data);
void getMAC(char *addr, uint8_t* data, uint16_t offset);
void channelHop(void* arg);

#endif // SNIFFER_H