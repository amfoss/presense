#include "main.cpp"
#include <Ticker.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <config.h>

HTTPClient http;
WiFiClientSecure client;
Ticker ticker;


const char* ssid = "";
const char* password = "";
// const char* graphql_endpoint_main = "https://notarobot.pythonanywhere.com/graphql";
const char* graphql_endpoint_main = "graphql_endpoint";
const char* secretKey = "your_secret_key";

void disablePromiscuousMode() {
  wifi_promiscuous_enable(DISABLE);
  os_timer_disarm(&channelHop_timer);
}

void enablePromiscuousMode() {
  wifi_promiscuous_enable(ENABLE);
  os_timer_arm(&channelHop_timer, CHANNEL_HOP_INTERVAL_MS, 1);
}

void setup() {
  Serial.begin(115200);
  client.setInsecure();
  
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

static void sendToServer() {
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

  String graphql_query_start = "{\"query\": \"mutation MyMutation { markAttendance(macList: \\\"";
  String graphql_query_middle = "\\\", secretKey: \\\"";
  String graphql_query_end = "\\\") { date id memberId }}\"}";
  
  String macListString = "";
  for (int i = 0; i < foundMacAddresses.size(); i++) {
    macListString += foundMacAddresses[i].c_str();
    if (i < foundMacAddresses.size() - 1) {
      macListString += ",";
    }
  }

  String graphql_query = graphql_query_start + macListString + graphql_query_middle + secretKey + graphql_query_end;

  Serial.println("GraphQL Query:");
  Serial.println(graphql_query);
  
  if (WiFi.status() == WL_CONNECTED) {
    // String graphql_query = "{\"query\": \"mutation MyMutation { markAttendance(macList: \"\", secretKey: \"\") { date id memberId }}\"}";
    

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

void loop() {
  sendToServer();
  delay(60000);
}
