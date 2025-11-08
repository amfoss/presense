#ifndef MEMBER_H
#define MEMBER_H

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include <map>
#include <vector>

extern char dateStr[11];
extern HTTPClient http;
extern WiFiClientSecure client;
extern JsonDocument memberData;
extern std::map<String, String> memberMap;
extern std::map<String, String> hmacMap;
extern std::vector<std::string> foundMacAddresses;

void fetchMemberData();
void sendToServer();

#endif // MEMBER_H