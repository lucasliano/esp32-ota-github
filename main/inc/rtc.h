/**
 * @file rtc.h
 * @brief Definition of RTC related functions.
 */

#ifndef RTC_H
#define RTC_H

// --- Generic includes ---
#include "common.h"
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"
#include "esp_netif_sntp.h"
#include "lwip/ip_addr.h"
#include "esp_sntp.h"

// --- Project related includes ---


// --- Generic defines ---
#define CONFIG_SNTP_TIME_SERVER "pool.ntp.org"
#define TIME_UPDATE_MS 10000


// --- Function definitions ---
esp_err_t rtc_sntp_init(void);
static void obtain_time(void);
void adjust_time_task(void *pvParameter);

#endif // RTC_H