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
#include "ble_task.h"
#include "victron_ble.h"
#include "victron_crypto.h"
#include "victron_parser.h"
#include "metrics_task.h"
#include "udp_client.h"
#include "vprintf.h"
#include "bme280.h"

// --- Generic defines ---
#define HASH_LEN 32



// --- Function definitions ---
esp_err_t system_health_checkup(void);


#endif // MAIN_H
