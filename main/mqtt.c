#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "main.h"
#include "gauge.h"
#include "rtc_wake_stub_example.h"



esp_mqtt_client_handle_t mqtt_client = NULL;
EventGroupHandle_t mqtt_event_group;
static int qos_test = 1;

const static int CONNECTED_BIT = BIT0;

void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
  esp_mqtt_event_t *data = (esp_mqtt_event_t *)event_data;
  switch (event_id) {
    case MQTT_EVENT_CONNECTED:
      ESP_LOGI("mqtt", "MQTT_EVENT_CONNECTED\n");
      xEventGroupSetBits(mqtt_event_group, CONNECTED_BIT);
      break;

    case MQTT_EVENT_DISCONNECTED:
      ESP_LOGI("mqtt", "MQTT_EVENT_DISCONNECTED\n");
      break;

    case MQTT_EVENT_SUBSCRIBED:
      ESP_LOGI("mqtt", "MQTT_EVENT_SUBSCRIBED, msg_id=%d\n", data->msg_id);
      break;

    case MQTT_EVENT_UNSUBSCRIBED:
      ESP_LOGI("mqtt", "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d\n", data->msg_id);
      break;

    case MQTT_EVENT_PUBLISHED:
      ESP_LOGI("mqtt", "MQTT_EVENT_PUBLISHED, msg_id=%d\n", data->msg_id);
      break;

    case MQTT_EVENT_DATA:
      ESP_LOGI("mqtt", "MQTT_EVENT_DATA\n");
      ESP_LOGI("mqtt", "TOPIC=%.*s\r\n", data->topic_len, data->topic);
      ESP_LOGI("mqtt", "DATA=%.*s\r\n", data->data_len, data->data);
      ESP_LOGI("mqtt", "ID=%d, total_len=%d, data_len=%d, current_data_offset=%d\n", data->msg_id, data->total_data_len, data->data_len, data->current_data_offset);
      // count++;
      break;

    case MQTT_EVENT_ERROR:
      ESP_LOGI("mqtt", "MQTT_EVENT_ERROR\n");
      break;

    case MQTT_EVENT_BEFORE_CONNECT:
      ESP_LOGI("mqtt", "MQTT_EVENT_BEFORE_CONNECT\n");
      break;

    default:
      ESP_LOGI("mqtt", "Other event id:%d\n", data->event_id);
      break;
  }
}

void start_mqtt(void) {
  esp_mqtt_client_config_t mqtt_cfg = {};
  mqtt_cfg.broker.address.hostname = MQTT_BROKER;
  mqtt_cfg.broker.address.port = 1883;
  mqtt_cfg.broker.address.transport = MQTT_TRANSPORT_OVER_TCP;
  mqtt_cfg.session.protocol_ver = MQTT_PROTOCOL_V_3_1_1;
  mqtt_cfg.credentials.username = "JWT";
  mqtt_cfg.network.timeout_ms = 30000;
  mqtt_cfg.credentials.authentication.password = DEVICE_KEY;

  ESP_LOGI("mqtt", "[APP] Free memory: %d bytes", esp_get_free_heap_size());
  mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
  esp_mqtt_client_register_event(mqtt_client, MQTT_EVENT_ANY, mqtt_event_handler, NULL);

  mqtt_event_group = xEventGroupCreate();
  esp_mqtt_client_start(mqtt_client);
  ESP_LOGI("mqtt", "Note free memory: %d bytes", esp_get_free_heap_size());
  ESP_LOGI("mqtt", "Waiting for connection to MQTT\n");
  xEventGroupWaitBits(mqtt_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
  ESP_LOGI("mqtt", "Connected to MQTT\n");
}

void sendPIReventToMQTT(void) {
  time_t now = 0;

  char msg[150];
  time(&now);

  const char* room_id;

  if (DEVICE_ID == "2") {
    room_id = "corridor";
  } else if (DEVICE_ID == "3") {
    room_id = "bathroom";
  }

  int size = snprintf(msg, sizeof(msg), "{\"sensors\":[{\"name\":\"PIR\",\"values\":[{\"timestamp\":%llu, \"roomID\":\"%s\"}]}]}", now * 1000, room_id);
  ESP_LOGI("mqtt", "Sent <%s> to topic %s", msg, DEVICE_TOPIC);
  auto err = esp_mqtt_client_publish(mqtt_client, DEVICE_TOPIC, msg, size, 1, 0);
  if (err == -1) {
    printf("Error while publishing to mqtt\n");
    ESP_LOGI("functions", "SendToMqttFunction terminated");
    return ESP_FAIL;
  }
}

void sendPIR2eventToMQTT(void) {
  time_t now = 0;

  char msg[150];
  time(&now);

  const char* room_id = "corridor";

  int size = snprintf(msg, sizeof(msg), "{\"sensors\":[{\"name\":\"kitchen\",\"values\":[{\"timestamp\":%llu, \"roomID\":\"%s\"}]}]}", now * 1000, room_id);
  ESP_LOGI("mqtt", "Sent <%s> to topic %s", msg, DEVICE_TOPIC);
  auto err = esp_mqtt_client_publish(mqtt_client, DEVICE_TOPIC, msg, size, 1, 0);
  if (err == -1) {
    printf("Error while publishing to mqtt\n");
    ESP_LOGI("functions", "SendToMqttFunction terminated");
    return ESP_FAIL;
  }
}

void sendBatteryStatusToMQTT(void) {
  time_t now = 0;

  char msg[150];
  time(&now);

  int size = snprintf(msg, sizeof(msg), "{\"sensors\":[{\"name\":\"battery\",\"values\":[{\"timestamp\":%llu, \"voltage\":%.1f, \"soc\":%.1f}]}]}", now * 1000, voltage, rsoc);
  ESP_LOGI("mqtt", "Sent <%s> to topic %s", msg, DEVICE_TOPIC);
  auto err = esp_mqtt_client_publish(mqtt_client, DEVICE_TOPIC, msg, size, 1, 0);
  if (err == -1) {
    printf("Error while publishing to mqtt\n");
    ESP_LOGI("functions", "SendToMqttFunction terminated");
    return ESP_FAIL;
  }
}

void sendMagneticSwitchEventToMQTT(void) {
  time_t now = 0;

  char msg[150];
  time(&now);

  int size = snprintf(msg, sizeof(msg), "{\"sensors\":[{\"name\":\"magneticSwitch\",\"values\":[{\"timestamp\":%llu, \"roomID\":\"door\"}]}]}", now * 1000);
  ESP_LOGI("mqtt", "Sent <%s> to topic %s", msg, DEVICE_TOPIC);
  auto err = esp_mqtt_client_publish(mqtt_client, DEVICE_TOPIC, msg, size, 1, 0);
  if (err == -1) {
    printf("Error while publishing to mqtt\n");
    ESP_LOGI("functions", "SendToMqttFunction terminated");
    return ESP_FAIL;
  }
}

void sendAllPIReventsToMQTT(void) {
    if (s_count == 0) {
        ESP_LOGI("mqtt", "No events to send.");
        return;
    }

     const char* room_id;

    if (DEVICE_ID == "2") {
    room_id = "corridor";
    } else if (DEVICE_ID == "3") {
    room_id = "bathroom";
  }

    char msg[512] = {0};  // Buffer for the JSON message
    char msgKitchen[512] = {0};  // Buffer for the JSON message
    char values[400] = {0};  // Buffer for the "values" array
    char valuesKitchen[400] = {0};  // Buffer for the "values" array

    // Initialize the JSON structures
    snprintf(msg, sizeof(msg), "{\"sensors\":[{\"name\":\"PIR\",\"values\":[");
    int addedPIR = 0;
    snprintf(msgKitchen, sizeof(msgKitchen), "{\"sensors\":[{\"name\":\"kitchen\",\"values\":[");
    int addedKitchen = 0;

    // Iterate through the event buffer and append each event to the "values" array
    for (int i = 0; i < s_count; i++) {
      if(event_buffer[i].sensor_id == 0) {
        ESP_LOGI("mqtt", "Event %d: room", i);
        char event[100];
        snprintf(event, sizeof(event),
                 "{\"timestamp\":%llu, \"roomID\":\"%s\"}%s",
                 event_buffer[i].timestamp * 1000,  // Convert to milliseconds
                 room_id, 
                 (({ int found = 0; for (int j = i + 1; j < s_count; j++) { if (j < s_count && event_buffer[j].sensor_id == 0) { found = 1; break; } } found; })
                 ) ? "," : "");  // Add a comma between events except for the last

        strncat(values, event, sizeof(values) - strlen(values) - 1);
        addedPIR = 1;
      } else {
        ESP_LOGI("mqtt", "Event %d: kitchen", i);
        char eventk[100];
        snprintf(eventk, sizeof(eventk),
                 "{\"timestamp\":%llu, \"roomID\":\"%s\"}%s",
                 event_buffer[i].timestamp * 1000,  // Convert to milliseconds
                 room_id,
                  (({ int found = 0; for (int j = i + 1; j < s_count; j++) { if (j < s_count && event_buffer[j].sensor_id == 1) { found = 1; break; } } found; })
                 ) ? "," : "");  // Add a comma between events except for the last

        strncat(valuesKitchen, eventk, sizeof(valuesKitchen) - strlen(valuesKitchen) - 1);
        addedKitchen = 1;
      }
    }

    // Publish the JSON message to MQTT
    if(addedPIR == 1) {

      // Complete the JSON message
      strncat(msg, values, sizeof(msg) - strlen(msg) - 1);
      strncat(msg, "]}]}", sizeof(msg) - strlen(msg) - 1);
      int err = esp_mqtt_client_publish(mqtt_client, DEVICE_TOPIC, msg, 0, 1, 0);
      ESP_LOGI("mqtt", "Sent <%s> to topic %s", msg, DEVICE_TOPIC);
      if (err <0) {
        ESP_LOGE("mqtt", "Error while publishing PIR events to MQTT.");
        return;
      }
    }
    if(addedKitchen == 1){

      // Complete the JSON message
      strncat(msgKitchen, valuesKitchen, sizeof(msg) - strlen(msgKitchen) - 1);
      strncat(msgKitchen, "]}]}", sizeof(msg) - strlen(msgKitchen) - 1);
      int err = esp_mqtt_client_publish(mqtt_client, DEVICE_TOPIC, msgKitchen, 0, 1, 0);
      ESP_LOGI("mqtt", "Sent <%s> to topic %s", msgKitchen, DEVICE_TOPIC);
      if (err < 0) {
        ESP_LOGE("mqtt", "Error while publishing kitchen events to MQTT.");
        return;
      }
    }
    
    

    // Clear the event buffer after sending
    s_count = 0;
}
