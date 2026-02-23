/**
 * @file main.h
 * @brief Definition main application functions and defines.
 */

#ifndef MAIN_H
#define MAIN_H

// --- Generic includes ---
#include "common.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <sys/socket.h>

// --- Project related includes ---
#include "ota.h"
#include "wifi.h"
#include "gpio.h"
#include "rtc.h"
#include "led_task.h"


// --- Generic defines ---
#define LOW_PRIORITY 1
#define MED_PRIORITY 2
#define HIGH_PRIORITY 3
#define CRITICAL_PRIORITY 4
#define APP_MINIMAL_STACK_SIZE 2048



// --- Function definitions ---
esp_err_t system_health_checkup(void);


#endif // MAIN_H
