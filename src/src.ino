#include "main.cpp"
#include <Ticker.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <config.h>
#include <ArduinoJson.h>
#include <map>
#include <iostream>

HTTPClient http;
WiFiClientSecure client;
Ticker ticker;


const char* ssid = "";
const char* password = "";
const char* graphql_endpoint_main = "https://fakeroot.shuttleapp.rs";
const char* secretKey = "your_secret_key";

const char* fetchQuery = "{\"query\": \"query fetch { getMember { id macaddress } }\"}";

StaticJsonDocument<2000> memberData;

std::map<String,String> memberMap;

void setup() {
  Serial.begin(115200);
  client.setInsecure();

  fetchMemberData();
  
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

  memberData.clear();
}

void loop() {
  delay(60000);
  sendToServer();
}

void disablePromiscuousMode() {
  wifi_promiscuous_enable(DISABLE);
  os_timer_disarm(&channelHop_timer);
}

void enablePromiscuousMode() {
  wifi_promiscuous_enable(ENABLE);
  os_timer_arm(&channelHop_timer, CHANNEL_HOP_INTERVAL_MS, 1);
}

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

  String graphql_query_start = "{\"query\": \"mutation batchAttendance {";
  String graphql_query_middle = "";
  // "\\\", secretKey: \\\"";
  String graphql_query_end = "}\"}";
  for (int i = 0; i < foundMacAddresses.size(); i++) {
    if (memberMap.find(foundMacAddresses[i].c_str()) != memberMap.end()) {
      graphql_query_middle += " a" + String(i) + ": addAttendance(id:"+memberMap[foundMacAddresses[i].c_str()] +",date:\\\"2024-08-15\\\",timein:\\\"09:00:00.123\\\",timeout:\\\"10:15:00.123\\\",isPresent:true){ id }";
    }
  }

  if (graphql_query_middle == "") {
    enablePromiscuousMode();
    return;
  }

  String graphql_query = graphql_query_start + graphql_query_middle + graphql_query_end;

  Serial.println("GraphQL Query:");
  Serial.println(graphql_query);
  
  if (WiFi.status() == WL_CONNECTED) {

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
  } else {
    Serial.println("Not connected to WiFi");
  }
  foundMacAddresses.clear();
  enablePromiscuousMode();
}
