/*
 * SPDX-FileCopyrightText: 2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include <inttypes.h>
#include "esp_sleep.h"
#include "esp_cpu.h"
#include "esp_rom_sys.h"
#include "esp_wake_stub.h"
#include "sdkconfig.h"

#include "esp_rom_sys.h"
#include "hal/rtc_io_ll.h"
#include "rom/rtc.h"
#include "soc/rtc.h"
#include "soc/rtc_periph.h"
#include "soc/sens_periph.h"
#include "soc/timer_group_reg.h"
#include "soc/uart_reg.h"
#include "esp_private/esp_clk.h"
#include "rtc_wake_stub_example.h"
#include "main.h"


/*
 * Deep sleep wake stub function is a piece of code that will be loaded into 'RTC Fast Memory'.
 * The first way is to use the RTC_IRAM_ATTR attribute to place a function into RTC memory,
 * The second way is to place the function into any source file whose name starts with rtc_wake_stub.
 * Files names rtc_wake_stub* have their contents automatically put into RTC memory by the linker.
 *
 * First, call esp_set_deep_sleep_wake_stub to set the wake stub function as the RTC stub entry,
 * The wake stub function runs immediately as soon as the chip wakes up - before any normal
 * initialisation, bootloader, or ESP-IDF code has run. After the wake stub runs, the SoC
 * can go back to sleep or continue to start ESP-IDF normally.
 *
 * Wake stub code must be carefully written, there are some rules for wake stub:
 * 1) The wake stub code can only access data loaded in RTC memory.
 * 2) The wake stub code can only call functions implemented in ROM or loaded into RTC Fast Memory.
 * 3) RTC memory must include any read-only data (.rodata) used by the wake stub.
 */

// maximum number of events to store
#define s_max_count 5

RTC_IRAM_ATTR uint32_t max_count = 5;

// counter value, stored in RTC memory
RTC_IRAM_ATTR uint32_t s_count = 0;

// couts wakeups, stored in RTC memory
RTC_IRAM_ATTR uint8_t wake_count = 0;

// wakeup_cause stored in RTC memory
static uint32_t wakeup_cause; // 0 = undefined, 1 = ext0, 2 = ext1, 3 = timer

// wakeup_time from CPU start to wake stub
static uint32_t wakeup_time;

// array to save the event in
RTC_IRAM_ATTR Event event_buffer[s_max_count];

// my_rtc_time_get_us() returns the rtc clock value in the wakeup stub
RTC_IRAM_ATTR uint64_t my_rtc_time_get_us(void) {
  SET_PERI_REG_MASK(RTC_CNTL_TIME_UPDATE_REG, RTC_CNTL_TIME_UPDATE);
  while (GET_PERI_REG_MASK(RTC_CNTL_TIME_UPDATE_REG, RTC_CNTL_TIME_VALID) == 0) {
    ets_delay_us(1);  // might take 1 RTC slowclk period, don't flood RTC bus
  }
  SET_PERI_REG_MASK(RTC_CNTL_INT_CLR_REG, RTC_CNTL_TIME_VALID_INT_CLR);
  uint64_t t = READ_PERI_REG(RTC_CNTL_TIME0_REG);
  t |= ((uint64_t)READ_PERI_REG(RTC_CNTL_TIME1_REG)) << 32;

  uint32_t period = REG_READ(RTC_SLOW_CLK_CAL_REG);

  // Convert microseconds to RTC clock cycles
  uint64_t now1 = ((t * period) >> RTC_CLK_CAL_FRACT);

  return now1;
}

void wake_stub_example(void)
{
    REG_WRITE(TIMG_WDTFEED_REG(0), 1);
    
    wakeup_time = esp_cpu_get_cycle_count() / esp_rom_get_cpu_ticks_per_us();
    wakeup_cause = esp_wake_stub_get_wakeup_cause();
    ESP_RTC_LOGI("wake stub: wakeup cause is %d", wakeup_cause);
    uint32_t timestamp = my_rtc_time_get_us();

    uint8_t wake_from_timer = 0;
    if(wakeup_cause != 1 && wakeup_cause != 2) {
      ESP_RTC_LOGI("wake stub: wakeup from timer every 10 minutes");
      if(s_count > 0 && timestamp - event_buffer[0].timestamp > 20*1000000) {
        ESP_RTC_LOGI("wake stub: At least one event has been stored for over 1h, waking up");
        wake_from_timer = 1;
      }
    }

    if(wakeup_cause == 1 || wakeup_cause == 2) {
      
        // add to the event buffer
        event_buffer[s_count].timestamp = timestamp;
        event_buffer[s_count].sensor_id = 0;
        ESP_RTC_LOGI("Wake stub: stored event at position %u", s_count);
        if (wakeup_cause == 2) {
          ESP_RTC_LOGI("Wake stub: ext1 wakeup, check for PIR2");
          uint64_t status = REG_READ(RTC_CNTL_EXT_WAKEUP1_STATUS_REG);

  
          if (status & BIT(9)) {
            ESP_RTC_LOGI("Wakeup caused by GPIO32 (RTC GPIO9)");
          }

          ESP_RTC_LOGI("Wake stub: RTC wakeup status: %u", status);
          // Prüfe GPIO32 (RTC GPIO9)
          if (status & BIT(9)) {
            ESP_RTC_LOGI("Wake stub: kitchen detected");
            event_buffer[s_count].sensor_id = 1;
            ESP_RTC_LOGI("Updated sensor id to 1");
          }
        }
        ESP_RTC_LOGI("Stub has detected event number %u", s_count);
        s_count++;
        ESP_RTC_LOGI("next event will be %u ", s_count);
  
    // check if the last event is older than 5 minutes
    if (s_count >= 2 
     && event_buffer[s_count - 1].sensor_id == event_buffer[s_count - 2].sensor_id
     && event_buffer[s_count - 1].timestamp - event_buffer[s_count - 2].timestamp < 5*60*1000000) {
        ESP_RTC_LOGI("Wake stub: last event is too recent, discarding event");
        s_count--;
      }
    }

    
    ESP_RTC_LOGI("wake stub: wakeup count is %d, wakeup cause is %d, wakeup cost %ld us, RTC clock: %llu", s_count, wakeup_cause, wakeup_time,timestamp/1000000);

    if (s_count >= max_count || wake_from_timer) {
        ESP_RTC_LOGI("wake stub: event buffer full, waking up");
        esp_default_wake_deep_sleep();
        return;
    }
    // go back to deep sleep.

    // Set wakeup time in stub, if need to check GPIOs or read some sensor periodically in the stub.
    esp_wake_stub_set_wakeup_time(10*1000000);

    ESP_RTC_LOGI("wake stub: going to deep sleep");
    esp_wake_stub_sleep(&wake_stub_example);
    //esp_deep_sleep_start();
}
