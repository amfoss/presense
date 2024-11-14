#include "main.cpp"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <config.h>
#include <ArduinoJson.h>
#include <map>
#include <iostream>
#include "time.h"
#include "mbedtls/md.h"
#include <esp_wpa2.h>

HTTPClient http;
WiFiClientSecure client;

// Time-related variables
const char* timeApiUrl = "http://api.timezonedb.com/v2.1/get-time-zone?key=API_KEY&format=json&by=zone&zone=Asia/Kolkata";
const long gmtOffset_sec = 19800;
const int daylightOffset_sec = 0;
struct tm timeinfo;
char dateStr[11];

const char* ssid = "";
const char* password = "";
const char* username = "";
const char* graphql_endpoint_main = "https://root.shuttleapp.rs";
const char* secretKey = "";

const char* fetchQuery = "{\"query\": \"query fetch { getMember { id macaddress } }\"}";

// Member data-related variables
StaticJsonDocument<2000> memberData;
std::map<String, String> memberMap;
std::map<String, String> hmacMap;

// Timer for channel hopping
esp_timer_handle_t channelHop_timer;

uint8_t newMACAddress[] = {0xD0, 0xAB, 0xD5, 0x22, 0xBE, 0x56};

// Setup function
void setup() {
    Serial.begin(115200);
    client.setInsecure();

    WiFi.mode(WIFI_STA); 

    Serial.print("DEFAULT MAC Address: ");
    readMacAddress();

    esp_err_t err = esp_wifi_set_mac(WIFI_IF_STA, &newMACAddress[0]);
    if (err == ESP_OK) {
        Serial.println("Mac Address Changed");
    }

    Serial.print("Spoofed MAC Address: ");
    readMacAddress();
    
    // WPA2 Enterprise PEAP setup
    esp_wifi_sta_wpa2_ent_set_identity((uint8_t*)username, strlen(username));
    esp_wifi_sta_wpa2_ent_set_username((uint8_t*)username, strlen(username));
    esp_wifi_sta_wpa2_ent_set_password((uint8_t*)password, strlen(password));
    
    esp_wifi_sta_wpa2_ent_enable(); 

    fetchMemberData();

    struct tm currentTime = getTimeInfo();
    strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", &currentTime);
    Serial.println("\nDate: " + String(dateStr));

    createHMACMap();

    // Set the WiFi chip to promiscuous mode aka monitor mode
    delay(10);
    WiFi.mode(WIFI_STA);

    esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
    esp_wifi_set_promiscuous_rx_cb(&sniffer_callback);
    esp_wifi_set_promiscuous(true);

    // Setup channel hopping timer
    const esp_timer_create_args_t timer_args = {
        .callback = &channelHop,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "channel_hop"
    };
    esp_timer_create(&timer_args, &channelHop_timer);
    esp_timer_start_periodic(channelHop_timer, 500 * 1000); // 500ms interval
}

void loop() {
    delay(90000);
    sendToServer();
}

struct tm getTimeInfo() {
    struct tm timeInfo = {0};

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi is not connected");
        return timeInfo;
    }

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
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, response);

        const char* dateTime = doc["formatted"];
        Serial.println(dateTime);
        strptime(dateTime, "%Y-%m-%d %H:%M:%S", &timeInfo);

        http.end();
    } else {
        Serial.println("Failed to fetch time : " + String(httpResponseCode));
        http.end();
    }

    return timeInfo;
}


void readMacAddress(){
    uint8_t baseMac[6];
    esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
    if (ret == ESP_OK) {
        Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
        baseMac[0], baseMac[1], baseMac[2],
        baseMac[3], baseMac[4], baseMac[5]);
    } else {
        Serial.println("Failed to read MAC address");
    }
}

// Function to fetch member data from server and create a map of mac addresses and member IDs
void fetchMemberData() {
    WiFi.begin(ssid);
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
    String response = http.getString();
    Serial.println(response);

    deserializeJson(memberData, response);
    http.end();

    for (JsonVariant member : memberData["data"]["getMember"].as<JsonArray>()) {
        String macAddress = member["macaddress"].as<String>();
        macAddress.toLowerCase();
        String memberId = member["id"].as<String>();
        memberMap[macAddress] = memberId;
    }

    for (auto const& x : memberMap) {
        Serial.print("MAC: ");
        Serial.print(x.first.c_str());
        Serial.print(" ID: ");
        Serial.println(x.second.c_str());
    }

    Serial.println("Member map created");
    Serial.println(memberMap.size());
}

// Function to create HMAC map
void createHMACMap() {
    Serial.print("HMAC date : ");
    Serial.println(dateStr);

    for (JsonVariant member : memberData["data"]["getMember"].as<JsonArray>()) {
        String memberId = member["id"].as<String>();
        String hmac = createHash(memberId, dateStr);
        hmacMap[memberId] = hmac;
    }

    Serial.print("HMAC map size : ");
    Serial.println(hmacMap.size());

    memberData.clear();
}

// Disable promiscuous mode
void disablePromiscuousMode() {
    esp_wifi_set_promiscuous(false);
    esp_timer_stop(channelHop_timer);
}

// Enable promiscuous mode
void enablePromiscuousMode() {
    esp_wifi_set_promiscuous(true);
    esp_timer_start_periodic(channelHop_timer, 500 * 1000);
}

// Function to send data to server
void sendToServer() {
    Serial.print("Found Mac Addresses size: ");
    Serial.println(foundMacAddresses.size());

    if (foundMacAddresses.size() == 0) {
        return;
    }

    disablePromiscuousMode();
    delay(10);

    
    while (WiFi.status() != WL_CONNECTED) {
        WiFi.begin(ssid);
        Serial.print("-");
        delay(10000);
    }

    Serial.println("");
    Serial.println("Connected to WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    int batchSize = 1;
    Serial.print("Batch size: ");
    Serial.println(batchSize);

    if (WiFi.status() == WL_CONNECTED) {
        for (int i = 0; i < foundMacAddresses.size(); i++) {
            String graphql_query = "{\"query\": \"mutation batchAttendance {";
            String macAddress = foundMacAddresses[i].c_str();
            String member_id = memberMap[macAddress];

            Serial.print("Member ID: ");
            Serial.println(member_id);
            if (!member_id.isEmpty()) {
                graphql_query += " a" + String(i) + ": markAttendance(id:" + member_id + ",date:\\\"" + dateStr + "\\\",isPresent:true,hmacSignature:\\\"" + hmacMap[member_id] + "\\\"){ id }";
            }
            // for (int j = 0; j < batchSize && i + j < foundMacAddresses.size(); j++) { }

            graphql_query += "}\"}";

            if (graphql_query == "{\"query\": \"mutation batchAttendance {}\"}") {
                Serial.println("No data to send");
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

// Function to create HMAC SHA256 hash
String createHash(String id, String dateStr) {
    Serial.println("Creating hash");

    const size_t keyLength = strlen(secretKey);
    const size_t messageLength = id.length() + dateStr.length() + 4;

    uint8_t hash[32];
    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;
    const mbedtls_md_info_t* md_info = mbedtls_md_info_from_type(md_type);

    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, md_info, 1);
    mbedtls_md_hmac_starts(&ctx, (const unsigned char*)secretKey, keyLength);

    String message = id + dateStr + "true";
    mbedtls_md_hmac_update(&ctx, (const unsigned char*)message.c_str(), message.length());
    mbedtls_md_hmac_finish(&ctx, hash);
    mbedtls_md_free(&ctx);

    String result;
    for (int i = 0; i < sizeof(hash); i++) {
        if (hash[i] < 16) result += '0';
        result += String(hash[i], HEX);
    }

    Serial.println("\nHash created");
    return result;
}
