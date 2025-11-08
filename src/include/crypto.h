#ifndef CRYPTO_H
#define CRYPTO_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <map>

extern const char* secretKey;
extern char dateStr[11];
extern JsonDocument memberData;
extern std::map<String, String> hmacMap;

String createHash(String id, String dateStr);
void createHMACMap();

#endif // CRYPTO_H