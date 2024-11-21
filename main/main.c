#include "main.h"

#include <stdio.h>
#include <string.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gauge.h"
#include "mqtt.h"
#include "nvs_flash.h"
#include "sntp.h"
#include "wifi.h"
#include "esp_sleep.h"
#include "esp_mac.h"
#include "esp_pm.h"
#include "rtc_wake_stub_example.h" 

int count = 0;

void IRAM_ATTR handlePIRevent(void *arg) {
  count++;
  ets_printf("Got PIR event %d\n", count);
}


void app_main() {
  //configPM();
  ESP_LOGI("progress", "[APP] Free memory: %d bytes", esp_get_free_heap_size());
  ESP_LOGI("progress", "[APP] IDF version: %s", esp_get_idf_version());

  esp_log_level_set("*", ESP_LOG_INFO);
  esp_log_level_set("mqtt", ESP_LOG_INFO);
  esp_log_level_set("progress", ESP_LOG_INFO);
  esp_log_level_set("gauge", ESP_LOG_INFO);

  // get current MAC address and log it
  uint8_t current_mac[6];
  esp_read_mac(current_mac, ESP_MAC_WIFI_STA);
  ESP_LOGI("progress", "MAC address: %02x:%02x:%02x:%02x:%02x:%02x",
             current_mac[0], current_mac[1], current_mac[2],current_mac[3], current_mac[4], current_mac[5]);

  // MAC adress for bathroom: ec:62:60:bc:e8:b4
  const uint8_t BATHROOM_MAC[6] = {0xEC, 0x62, 0x60, 0xBC, 0xE8, 0xB4};

  // MAC adress for corridor: ec:62:60:bc:e7:c0
  const uint8_t CORRIDOR_MAC[6] = {0xEC, 0x62, 0x60, 0xBC, 0xE7, 0xC0};

  // MAC adress for door: 94:3c:c6:c2:73:c8
  const uint8_t DOOR_MAC[6] = {0x94, 0x3C, 0xC6, 0xC2, 0x73, 0xC8};

  if(memcmp(current_mac, DOOR_MAC, 6) != 0) {
    // Doorsensor does not have a battery gauge
    getRSOC();
  }

  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  ESP_LOGI("progress", "This is the main application");

  ESP_LOGI("progress", "Starting Wifi");
  start_wifi();

  ESP_LOGI("progress", "Starting Clock");
  start_clock();

  ESP_LOGI("progress", "Starting MQTT");
  start_mqtt();

  if(memcmp(current_mac, BATHROOM_MAC, 6) == 0 | memcmp(current_mac, CORRIDOR_MAC, 6) == 0) {
    // only send every 10th battery status to MQTT
    if(count % 10 == 0) {
      ESP_LOGI("progress", "Sending battery status to MQTT");
      sendBatteryStatusToMQTT();
    }
  }

  // if mac address is for corridor/bathroom, send PIR event; if mac address is for door, send magnetic switch event
  if( memcmp(current_mac, CORRIDOR_MAC, 6) == 0) {
    //esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    //if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT1) {
    //    uint64_t wakeup_mask = esp_sleep_get_ext1_wakeup_status();
    //    if ((wakeup_mask & (1ULL << PIR_PIN)) != 0) {
    //        ESP_LOGI("progress", "Wakeup from PIR_PIN room");
    //        ESP_LOGI("progress", "Sending PIR event to MQTT");
    //        sendPIReventToMQTT();

    //    } else if ((wakeup_mask & (1ULL << PIR_PIN2)) != 0) {
    //        ESP_LOGI("progress", "Wakeup from PIR_PIN2 kitchen");
            //DEVICE_TOPIC = "1/2/kitchen";
    //        ESP_LOGI("progress", "Sending PIR event to MQTT");
    //        sendPIR2eventToMQTT();

    //    }
    //} else {
    //    ESP_LOGI("progress", "Power-on or other reset, not EXT1 wakeup");
    //}

    ESP_LOGI("progress", "Sending PIR events to MQTT");
    sendAllPIReventsToMQTT();

    ESP_LOGI("progress", "The connected device is a PIR sensor");
    
    ESP_ERROR_CHECK(gpio_set_direction(PIR_PIN, GPIO_MODE_INPUT));
    ESP_ERROR_CHECK(rtc_gpio_pulldown_en(PIR_PIN));
    ESP_ERROR_CHECK(gpio_set_direction(PIR_PIN2, GPIO_MODE_INPUT));
    ESP_ERROR_CHECK(rtc_gpio_pulldown_en(PIR_PIN2));

    // Schleife zum Warten, bis beide Pins auf LOW sind
    while (gpio_get_level(PIR_PIN) == 1 || gpio_get_level(PIR_PIN2) == 1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    ESP_LOGI("progress", "Installing wakeup");
    // EXT1 Wakeup auf beide Pins setzen
    ESP_ERROR_CHECK(esp_sleep_enable_ext1_wakeup(PIR_PIN_MASK, ESP_EXT1_WAKEUP_ANY_HIGH));

    ESP_LOGI("progress", "Going to sleep");
    esp_set_deep_sleep_wake_stub(&wake_stub_example);
    esp_deep_sleep_start();

  } else if (memcmp(current_mac, BATHROOM_MAC, 6) == 0) {
    ESP_LOGI("progress", "The connected device is a PIR sensor");
    ESP_LOGI("progress", "Sending PIR event to MQTT");
    //sendPIReventToMQTT();
    if (s_count == 0) {
        ESP_LOGI("mqtt", "No events to send.");
    } else {
        ESP_LOGI("progress", "Sending %u PIR event to MQTT", s_count);
        sendAllPIReventsToMQTT();
        s_count = 0;
    }

    ESP_ERROR_CHECK(gpio_set_direction(PIR_PIN, GPIO_MODE_INPUT));
    ESP_ERROR_CHECK(rtc_gpio_pulldown_en(PIR_PIN));

    ESP_LOGI("progress", "Installing wakeup");
    while (gpio_get_level(PIR_PIN)==1){
      vTaskDelay(pdMS_TO_TICKS(1000));
    }

    ESP_ERROR_CHECK(esp_sleep_enable_ext0_wakeup(PIR_PIN, 1));
    ESP_LOGI("progress", "Going to sleep");
    esp_set_deep_sleep_wake_stub(&wake_stub_example);
    esp_deep_sleep_start();

  } else if(memcmp(current_mac, DOOR_MAC, 6) == 0) {

    ESP_LOGI("progress", "The connected device is a door sensor");
    ESP_LOGI("progress", "Sending magnetic switch event to MQTT");
    ESP_LOGI("progress", "GPIO level: %d", gpio_get_level(SWITCH_PIN));
    sendMagneticSwitchEventToMQTT();

    ESP_ERROR_CHECK(gpio_set_direction(SWITCH_PIN, GPIO_MODE_INPUT));
    ESP_ERROR_CHECK(rtc_gpio_pulldown_en(SWITCH_PIN));

    ESP_LOGI("progress", "Installing wakeup");
    while (gpio_get_level(SWITCH_PIN)==1){
      vTaskDelay(pdMS_TO_TICKS(1000));
    }

    ESP_ERROR_CHECK(esp_sleep_enable_ext0_wakeup(SWITCH_PIN, 1));
   
    ESP_LOGI("progress", "Going to sleep");
    //esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
    esp_deep_sleep_start();

  } else {
    ESP_LOGI("progress", "Unknown device");
  }

}

//void configPM() {
//  ESP_LOGI("progress", "Configuring LIGHT SLEEP");
//  esp_pm_config_esp32_t pm_config = {
//    .max_freq_mhz = 160,
//    .min_freq_mhz = 160,
//    .light_sleep_enable = true
//  };
//  ESP_ERROR_CHECK(esp_pm_configure(&pm_config));
//}