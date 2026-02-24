/**
 * @file wtd.h
 * @brief Tarea FreeRTOS para controlar LED onboard.
 */

#ifndef WTD_H
#define WTD_H


// --- Generic includes ---
#include "common.h"
#include "rtc_wdt.h"

// --- Project related includes ---


// --- Generic defines ---
#define WTD_TIMER_MS 10000
#define WTD_TASK_MS 9000

// --- Function definitions ---
void wtd_task(void *pvParameter);
esp_err_t rtc_wtd_init(unsigned int);
esp_err_t rtc_wtd_config(unsigned int timeout_ms);
esp_err_t rtc_wtd_ok(void);



#endif // WTD_H
