#include "../include/member.h"

#include "../include/crypto.h"
#include "../include/network.h"
#include "../include/secrets.h"

const char *graphql_endpoint_main = GRAPHQL_ENDPOINT;
const char *fetchQuery =
    "{\"query\": \"query { allMembers { memberId macAddress } }\"}";

void fetchMemberData() {
  connectToWiFi();

  int status = 0;
  int retryCount = 0;

  http.begin(client, graphql_endpoint_main);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(15000);

  while (status <= 0) {
    Serial.println("Attempting to fetch member data (Retry " +
                   String(retryCount) + ")");
    status = http.POST(fetchQuery);
    Serial.println("Status: " + String(status));

    if (status <= 0) {
      retryCount++;
      delay(5000);
    }
  }

  Serial.println("Member data fetched");
  String response = http.getString();
  Serial.println(response);

  deserializeJson(memberData, response);
  http.end();

  for (JsonVariant member : memberData["data"]["allMembers"].as<JsonArray>()) {
    String macAddress = member["macAddress"].as<String>();
    macAddress.toLowerCase();
    String memberId = member["memberId"].as<String>();
    memberMap[macAddress] = memberId;
  }

  for (auto const &x : memberMap) {
    Serial.print("MAC: ");
    Serial.print(x.first.c_str());
    Serial.print(" ID: ");
    Serial.println(x.second.c_str());
  }

  Serial.println("Member map created");
  Serial.println(memberMap.size());
}

void sendToServer() {
  Serial.print("Found Mac Addresses size: ");
  Serial.println(foundMacAddresses.size());

  if (foundMacAddresses.size() == 0) {
    return;
  }

  disablePromiscuousMode();
  delay(10);

  connectToWiFi();

  int batchSize = 1;
  Serial.print("Batch size: ");
  Serial.println(batchSize);

  if (WiFi.status() == WL_CONNECTED) {
    for (int i = 0; i < foundMacAddresses.size(); i++) {
      String graphql_query = "{\"query\": \"mutation batchAttendance {";
      String macAddress = foundMacAddresses[i].c_str();
      String member_id = memberMap[macAddress];

      if (member_id != "") {
        Serial.print("\nMember ID: ");
        Serial.println(member_id);
        graphql_query +=
            " a" + String(i) +
            ": markAttendance(input:{memberId:" + String(member_id) +
            ",date:\\\"" + dateStr + "\\\",hmacSignature:\\\"" +
            hmacMap[member_id] + "\\\"}){ timeIn timeOut }";
      }

      graphql_query += "}\"}";

      if (graphql_query == "{\"query\": \"mutation batchAttendance {}\"}") {
        continue;
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
      }

      http.end();
    }
  } else {
    Serial.println("Not connected to WiFi");
  }

  foundMacAddresses.clear();
  enablePromiscuousMode();
}
