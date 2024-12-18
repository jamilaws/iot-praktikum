/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void wake_stub_example(void);
RTC_IRAM_ATTR uint64_t my_rtc_time_get_us(void);

// Event structure
typedef struct {
    uint64_t timestamp;  // Event timestamp
    uint8_t sensor_id;   // Sensor ID (0 = PIR, 1 = kitchen)
} Event;

// Extern declarations
extern RTC_DATA_ATTR Event event_buffer[];    // Array for storing events
extern RTC_DATA_ATTR uint32_t s_count;         // Number of events stored
extern RTC_DATA_ATTR Event lastsent_event;     // Last event sent to MQTT
extern RTC_DATA_ATTR uint32_t wake_count;       // Number of wakeups (counts up to ten for battery)
extern RTC_DATA_ATTR uint64_t real_deep_sleep_start_time;         // Time of last deep sleep start
extern RTC_DATA_ATTR uint64_t rtc_deep_sleep_start_time;           // Time of last deep sleep start on rtc clock

#ifdef __cplusplus
}
#endif
