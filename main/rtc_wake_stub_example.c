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

// wakeup_cause stored in RTC memory
static uint32_t wakeup_cause;

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


// wake up stub function stored in RTC memory
void wake_stub_example(void)
{
    // Get wakeup time.
    wakeup_time = esp_cpu_get_cycle_count() / esp_rom_get_cpu_ticks_per_us();
    // Get wakeup cause.
    wakeup_cause = esp_wake_stub_get_wakeup_cause();
    // save timestamp in array
    uint32_t timestamp = my_rtc_time_get_us();
    // add to the event buffer
    event_buffer[s_count].timestamp = timestamp;
    event_buffer[s_count].sensor_id = 0;
    if (wakeup_cause == ESP_SLEEP_WAKEUP_EXT1) {
        uint64_t wakeup_mask = esp_sleep_get_ext1_wakeup_status();
        if ((wakeup_mask & (1ULL << PIR_PIN2)) != 0) {
            event_buffer[s_count].sensor_id = 1;
        }
    }
    // Increment the counter.
    ESP_RTC_LOGI("Stub has detected event number %d", s_count);
    s_count++;
    // Print the counter value and wakeup cause.
    ESP_RTC_LOGI("wake stub: wakeup count is %d, wakeup cause is %d, wakeup cost %ld us, RTC clock: %llu", s_count, wakeup_cause, wakeup_time,timestamp/1000000);

    if (s_count >= max_count) {
        // Reset s_count
        //s_count = 0;

        // Set the default wake stub.
        // There is a default version of this function provided in esp-idf.
        esp_default_wake_deep_sleep();

        // Return from the wake stub function to continue
        // booting the firmware.
        return;
    }
    // s_count is < s_max_count, go back to deep sleep.

    // Set wakeup time in stub, if need to check GPIOs or read some sensor periodically in the stub.
    esp_wake_stub_set_wakeup_time(5*1000000);

    // Print status.
    ESP_RTC_LOGI("wake stub: going to deep sleep");

    // Set stub entry, then going to deep sleep again.
    esp_wake_stub_sleep(&wake_stub_example);
    esp_deep_sleep_start();
}
