#include "../include/time_utils.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>

#include "../include/network.h"
#include "../include/secrets.h"

const char *timeApiUrl =
    "http://api.timezonedb.com/v2.1/get-time-zone?key=" TIME_API_KEY
    "&format=json&by=zone&zone=Asia/Kolkata";

struct tm getTimeInfo() {
  struct tm timeInfo = {0};

  connectToWiFi();

  HTTPClient http;
  http.begin(timeApiUrl);
  int httpResponseCode = http.GET();

  while (httpResponseCode <= 0) {
    httpResponseCode = http.GET();
    Serial.println("Retrying to fetch time");
    delay(1000);
  }

  if (httpResponseCode == 200) {
    String response = http.getString();
    JsonDocument doc;
    deserializeJson(doc, response);

    const char *dateTime = doc["formatted"];
    Serial.println(dateTime);
    strptime(dateTime, "%Y-%m-%d %H:%M:%S", &timeInfo);

    http.end();
  } else {
    Serial.println("Failed to fetch time : " + String(httpResponseCode));
    http.end();
  }

  return timeInfo;
}