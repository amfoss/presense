#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_timer.h>
#include <esp_wpa2.h>
#include "../include/network.h"
#include "../include/secrets.h"

extern esp_timer_handle_t channelHop_timer;

const char* ssid = SSID;
const char* username = USERNAME;
const char* password = PASSWORD;
const bool is_enterprise = IS_ENTERPRISE;

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

void init_peap() {
    esp_wifi_sta_wpa2_ent_set_identity((uint8_t*)username, strlen(username));
    esp_wifi_sta_wpa2_ent_set_username((uint8_t*)username, strlen(username));
    esp_wifi_sta_wpa2_ent_set_password((uint8_t*)password, strlen(password));
    
    esp_wifi_sta_wpa2_ent_enable();
}

void connectToWiFi() {
    if (is_enterprise) {
        WiFi.begin(ssid);
    } else {
        WiFi.begin(ssid, password);
    }

    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);
    }

    Serial.println("\nConnected to WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

void disablePromiscuousMode() {
    esp_wifi_set_promiscuous(false);
    esp_timer_stop(channelHop_timer);
}

void enablePromiscuousMode() {
    esp_wifi_set_promiscuous(true);
    esp_timer_start_periodic(channelHop_timer, 500 * 1000);
}