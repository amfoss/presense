#include "main.cpp"
#include <Ticker.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <config.h>
#include <ArduinoJson.h>
#include <map>
#include <iostream>
#include "time.h"
#include "sha256.h"

HTTPClient http;
WiFiClientSecure client;
Ticker ticker;

// Time related variables
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;
const int daylightOffset_sec = 0;
struct tm timeinfo;
char dateStr[11];

// Network related variables
const char* ssid = "";
const char* password = "";
const char* graphql_endpoint_main = "https://fakeroot.shuttleapp.rs";
const char* secretKey = "";

const char* fetchQuery = "{\"query\": \"query fetch { getMember { id macaddress } }\"}";

// Member data related variables
StaticJsonDocument<2000> memberData;
std::map<String,String> memberMap;
std::map<String,String> hmacMap;

void setup() {
  Serial.begin(115200);
  client.setInsecure();

  fetchMemberData();

  while (!getLocalTime(&timeinfo)) {
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    Serial.print("+");
    delay(500);
  }

  Serial.println("\nDate:");
  strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", &timeinfo);
  Serial.println(dateStr);

  createHMACMap();

  // set the WiFi chip to "promiscuous" mode aka monitor mode
  delay(10);
  wifi_set_opmode(STATION_MODE);
  wifi_set_channel(1);
  wifi_promiscuous_enable(DISABLE);
  delay(10);
  wifi_set_promiscuous_rx_cb(sniffer_callback);
  delay(10);
  wifi_promiscuous_enable(ENABLE);
  
  // setup the channel hoping callback timer
  os_timer_disarm(&channelHop_timer);
  os_timer_setfn(&channelHop_timer, (os_timer_func_t *) channelHop, NULL);
  os_timer_arm(&channelHop_timer, CHANNEL_HOP_INTERVAL_MS, 1);

}

void loop() {
  delay(90000);
  sendToServer();
}

// Function to fetch member data from server and create a map of mac address and member id
void fetchMemberData() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("-");
  }
  int status = 0;
  http.begin(client, graphql_endpoint_main);
  http.addHeader("Content-Type", "application/json");
  while (status <= 0) {
    status = http.POST(fetchQuery);
    delay(1000);
  }
  Serial.println("Member data fetched");
  Serial.println(http.getString());

  deserializeJson(memberData, http.getString());
  http.end();

  for (int i = 0; i < memberData["data"]["getMember"].size(); i++) {
    String macAddress = memberData["data"]["getMember"][i]["macaddress"];
    String memberId = memberData["data"]["getMember"][i]["id"];
    memberMap[macAddress] = memberId;
  }

  Serial.println("Member map created");
  Serial.println(memberMap.size());
}

// Function to create hmac map
void createHMACMap() {
  Serial.print("HMAC date : ");
  Serial.println(dateStr);
  for (int i = 0; i < memberData["data"]["getMember"].size(); i++) {
    String memberId = memberData["data"]["getMember"][i]["id"];
    String hmac = createHash(memberId,dateStr);
    Serial.println(memberId);
    Serial.println(hmac);
    hmacMap[memberId] = hmac;
  }

  Serial.print("HMAC map size : ");
  Serial.println(hmacMap.size());

  memberData.clear();
}

// Diabling promiscuous mode
void disablePromiscuousMode() {
  wifi_promiscuous_enable(DISABLE);
  os_timer_disarm(&channelHop_timer);
}

// Enabling promiscuous mode
void enablePromiscuousMode() {
  wifi_promiscuous_enable(ENABLE);
  os_timer_arm(&channelHop_timer, CHANNEL_HOP_INTERVAL_MS, 1);
}

// Send data to server
// This function includes logic for creating the graphql queries
static void sendToServer() {
  Serial.print("Found Mac Addresses size: ");
  Serial.println(foundMacAddresses.size());
  if (foundMacAddresses.size() == 0) {
    return;
  }

  disablePromiscuousMode();
  delay(10);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("-");
  }

  Serial.println("");
  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  int batchSize = 4;
  Serial.print("Batch size: ");
  Serial.println(batchSize);
  if (WiFi.status() == WL_CONNECTED) {
    for (int i = 0; i < foundMacAddresses.size(); i+=batchSize) { 
      String graphql_query = "{\"query\": \"mutation batchAttendance {";
      for (int j = 0 ; j < batchSize && j < foundMacAddresses.size() ; j++) {
        if (memberMap.find(foundMacAddresses[i+j].c_str()) != memberMap.end()) { 
          String member_id = memberMap[foundMacAddresses[i+j].c_str()];
          graphql_query += " a" + String(i+j) + ": markAttendance(id:"+ member_id +",date:\\\""+dateStr+"\\\",isPresent:true,hmacSignature:\\\""+ hmacMap[member_id] +"\\\"){ id }";
        }
      }

      graphql_query += "}\"}";

      if (graphql_query == "{\"query\": \"mutation batchAttendance {}\"}") {
        enablePromiscuousMode();
        return;
      }

      Serial.println("GraphQL Query:");
      Serial.println(graphql_query);

      http.begin(client, graphql_endpoint_main);
      http.addHeader("Content-Type", "application/json");

      int httpResponseCode = http.POST(graphql_query);
      
      if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.print("HTTP Response Code: ");
        Serial.println(httpResponseCode);
        Serial.println("Response:");
        Serial.println(response);
      } else {
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
        Serial.print("HTTP Error Message: ");
        Serial.println(http.errorToString(httpResponseCode).c_str());
      }

      http.end();
    }
  } else {
    Serial.println("Not connected to WiFi");
  }
  foundMacAddresses.clear();
  enablePromiscuousMode();
}

// Function to convert string to uint8_t
uint8_t* convertToUint8(const char* str) {
    size_t len = strlen(str);
    uint8_t* uint8Array = new uint8_t[len];
    for (size_t i = 0; i < len; i++) {
        uint8Array[i] = static_cast<uint8_t>(str[i]);
    }
    return uint8Array;
}

// Function to create SHA256 hmac hash
String createHash(String id,String dateStr) {
  Serial.println("Creating hash");
  uint8_t* secret = convertToUint8(secretKey);
  uint8_t* hash;
  Sha256 sha;

  String message = id;
  message += dateStr;
  message += "true";

  sha.initHmac(secret, strlen(secretKey));

  yield();

  sha.print(message);
  hash = sha.resultHmac();

  String result = ""; 
  for (int i = 0; i < 32; i++) {
      if (hash[i] < 16) {
          result += '0';
      }
      String byteString = String(hash[i], HEX);
      byteString.toLowerCase();
      result += byteString;
  }

  delete[] secret;
  Serial.println("\nHash created");
  return result;
}
