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

char* DEVICE_ID = "2";
char* DEVICE_TOPIC = "1/2/data";
char* DEVICE_KEY = "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9.eyJpYXQiOjE3Mjk4NDQ5MjIsImlzcyI6ImlvdHBsYXRmb3JtIiwic3ViIjoiMS8yIn0.wF86Yw56TPwJG8OTvpGtGKZecU0ZipQbcr7QjgpZUIVC45pLepo2dO9Ad6IUB_w8g_ijvHOzlcgs1KlAv9ttrI3EDnuQrSBXhTJCLhK0kZHZwk1VKQxUJb4_UBlW1V9b7iCM8FK_aRfSleml_I5eFhqZIWAO5cnPEdCuPN0rJ0NsdhH047AgagAVvwIwZn5bViEjVuWGJZ6h2v-RGYaHWw_MYiGNbty04TpNWs2ZRPVxTCDJIkrO41iNbWjddrQOPFB4PhVCpfcZDzOnE5-lFAWZ6nptfI28C_JxIFkpOtiCHZLBRV8V-zl2_-GGn4fVuID6MZ8t1dVbxe_bQgqyBICdSa2oafzqhqtaXHUxlAL8omDPhIM_-2vKf6yGmb4LHqWrqMo_RrWPDBQb5JGmD1kk3qH7zTmkhzEB2oKPzxuvBvpgNYIOW7c6N1ooUNySphDy8Ry1rFNf8Xvmq2o_HmSuqdGQLuf25CJP8skdr4FooRzpkdZf-yHGPHPxH21donZ5TYYgKhfgnvQ5SZC-u3T-vlBizqOSjkyHhi-_gSnY2X9_Y7MeftPZJIHv0jw8YZXuje88vz2V1K1Hby1llZ7lEnldePgsTnGf8BweYxrR5Wq5dzX_ZlFN2sFQlrLMWlvshsdzLP9hQKNpwgOuu5IL857KkCUkRX58g6zuZXA";


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

  const uint8_t BATHROOM_MAC[6] = {0xEC, 0x62, 0x60, 0xBC, 0xE8, 0xB4};
  const uint8_t CORRIDOR_MAC[6] = {0xEC, 0x62, 0x60, 0xBC, 0xE7, 0xC0};
  const uint8_t DOOR_MAC[6] = {0x94, 0x3C, 0xC6, 0xC2, 0x73, 0xC8};

  // set DEVICE ID, DEVICE KEY and DEVICE TOPIC based on MAC address
  if(memcmp(current_mac, BATHROOM_MAC, 6) == 0) {
    ESP_LOGI("progress", "The connected device is a bathroom sensor");
    DEVICE_ID = "3";
    DEVICE_TOPIC = "1/3/data";
    DEVICE_KEY = "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9.eyJpYXQiOjE3MzA0OTAwNzIsImlzcyI6ImlvdHBsYXRmb3JtIiwic3ViIjoiMS8zIn0.gtQ1iVRv20Pz-YQmVfnGYAEYadGtQkAINfb9NKBt3IqGn0Tf_6NM3Idlvh9r5zh3Q-DSun393Q1qmQ7nSbvsMsA32KLcUI4RtjiL6Tbgbkc6pFbgk4Pie0J95lyLKjOuINit_wAIH8qK_jGRBA24guGdqd7AVERJzhvrJurOe2GtNENuT47BvkgnftCQx1W5M5SRq8QhW8rgRQTr7XNmTLHA2t1wxuTqGlzOLmKpzOHdYnucJzUvwKpZxaBex3ywqy9ZlJO4rmkJ9A5RlwG4791394tb6W-W0J77xFTUVWJ93waE2ro2q8Oyx_DxGZTKWq59L_3rJ-OMRafQfch73U5J7QKANfkq9dGGMSWSRnJFOQ3Nd0hRrWGhpUZTJSDdZfvYyThWFzZy91C00dWmN1K9cQ_Ld77gjReHB6t8OAKB4nPh6hHNOTyXWp48CcqZhsYl4uJQkLd2eanCEukRUn1-uug36ZDYsU8kfO84cNqILtz056FnDFWIAMforAjKRYIqJo-RcNn2kYwQeZq29NVqn5sdd-56n0aSk9GKUiR_nVP5u32BTHx2qmMkj-mmjs5BpOBbWls4Z8DBWI6o8D52a1gsDr_KIAhzMDqp1wtILgUTY-fpK7p-zaiNiGXZ6Rvcmm1CExwlLZyD0jgo2kBjraOYDPdC6kA8r4ds7vQ";
  } else if(memcmp(current_mac, DOOR_MAC, 6) == 0) {
    ESP_LOGI("progress", "The connected device is a door sensor");
    DEVICE_ID = "4";
    DEVICE_TOPIC = "1/4/data";
    DEVICE_KEY = "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9.eyJpYXQiOjE3MzA2Mzg2MDQsImlzcyI6ImlvdHBsYXRmb3JtIiwic3ViIjoiMS80In0.O9Uc5fiw12V46WMUnoFhi5bP-9jAOZVV6sFbm37t9paCo1rpYOiYOMnA4rqfJVW6FOlD3aZs_xP5WHFjJ_K-pesORigaHgw9NqN1K9gGM78GJV-dbki_wsyma_JyhFBPhuAa3fnH_ppqir1ev6X3auJJP-NdMcbAcjskX17TGp7XN3_6jkgR1ms2_2e_8FGbhtb_2d7EZ4xswzpIomj5QweW64hlJ7ExxJjP5pPVovfRMP5i2TR-ISmQ33cYh_cuABFzR5gnebkE52_HQHedK1JFjjcRSTfR7RI22XfKrPxi9dN3vsgjX9Iqs0kMcfWPzgFCrUWbHBMoVNrqed8-O8MJHAmMW8H7zzOlyadZSh9edCC5VDrR-kIzsARarVBbzhnh4IzSyS7GM1rjtmglpMTDyfXSgKPzHzUPUp9oSiNnmFcH67a0KTRQPNmfNxGYbZOGpKaXFbEYQl4dVoMEvhUb8S1a6g0GUu03qROfZx_g4tD6IT6LBOqNgDVLmNwM3ivqgIKWMmy3qg-aHoPl4Jn1ZfKOYu-aP1t3LkdM8Fbpi2XbtUMzR2aD2lmNuEWm6Dp5iB96gfqG-j0HWynRyw3UzZnANIyzqrYrFoxzOM2Qngjh15jIMQpVx_G5eX4gRoTdpqw2blS1zbTRSinmu978Q0EJHaWJ6tQpfF_7U8M";
  }

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
    ESP_LOGI("progress", "The wake up count is %d", count);
    if(wake_count >= 10) {
      ESP_LOGI("progress", "Sending battery status to MQTT");
      sendBatteryStatusToMQTT();
      wake_count = 0;
    } else {
      wake_count++;
      ESP_LOGI("progress", "No battery status sent. The wake up count now is %d", wake_count);
    }
  }

  // if mac address is for corridor/bathroom, send PIR event; if mac address is for door, send magnetic switch event
  if( memcmp(current_mac, CORRIDOR_MAC, 6) == 0) {

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
    ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(10*1000000));
    ESP_LOGI("progress", "Going to sleep");
    esp_set_deep_sleep_wake_stub(&wake_stub_example);
    esp_deep_sleep_start();

  // if mac address is for bathroom, send PIR event
  } else if (memcmp(current_mac, BATHROOM_MAC, 6) == 0) {
    ESP_LOGI("progress", "The connected device is a PIR sensor");
    ESP_LOGI("progress", "Sending PIR event to MQTT");
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
      vTaskDelay(pdMS_TO_TICKS(10000));
    }

    ESP_ERROR_CHECK(esp_sleep_enable_ext0_wakeup(PIR_PIN, 1));
    ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(10*1000000));
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