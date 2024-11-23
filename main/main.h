#pragma once

// SETUP SEMINAR ROOM
// #define EXAMPLE_ESP_WIFI_SSID      "CAPS-Seminar-Room"
// #define EXAMPLE_ESP_WIFI_PASS      "caps-schulz-seminar-room-wifi"
// #define SNTP_SERVER_NAME           "ntp1.in.tum.de"
// #define MQTT_BROKER                "131.159.85.91"

// SETUP HOME
#define EXAMPLE_ESP_WIFI_SSID      "FRITZ!Box 7430 TK"
#define EXAMPLE_ESP_WIFI_PASS      "87435946242696654701"
#define SNTP_SERVER_NAME           "pool.ntp.org"
#define MQTT_BROKER                "192.168.178.63"

// SETUP OFFICE
// #define EXAMPLE_ESP_WIFI_SSID       "CAPS"
// #define EXAMPLE_ESP_WIFI_PASS       "caps!schulz-wifi"
// #define SNTP_SERVER_NAME            "ntp1.in.tum.de"
// #define MQTT_BROKER                "131.159.85.91"

extern char* DEVICE_ID;
extern char* DEVICE_TOPIC;
extern char* DEVICE_KEY;

#define PIR_PIN 27 
#define SWITCH_PIN 32
#define PIR_PIN2 32
#define PIR_PIN_MASK ((1ULL<<PIR_PIN) | (1ULL<<PIR_PIN2))

