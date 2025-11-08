#include "config.h"

std::vector<std::string> foundMacAddresses;

/*
#include "config.h"
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";
const char* graphql_endpoint = "https://your-graphql-endpoint.com/graphql";


void sendGraphQLMutation() {
  WiFiClientSecure client;
  HTTPClient http;

  // Your GraphQL mutation
  String graphql_mutation = R"(
    {
      "query": "mutation { addMember(name: \"John Doe\", email: \"john.doe@example.com\") { id name email } }"
    }
  )";

  // Connect to the GraphQL endpoint
  if (client.connect(graphql_endpoint, 443)) {
    http.begin(client, graphql_endpoint);
    http.addHeader("Content-Type", "application/json");

    // Send the request
    int httpResponseCode  http.POST(graphql_mutation);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("Connection to GraphQL endpoint failed");
  }
}
*/
