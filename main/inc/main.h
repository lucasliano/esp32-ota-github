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
#include "uart.h"
#include "wtd.h"

// --- Generic defines ---




// --- Function definitions ---
esp_err_t system_health_checkup(void);


#endif // MAIN_H
