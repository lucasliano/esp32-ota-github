/**
 * @file common.h
 * @brief Generic definitions among various .h files.
 */

#ifndef COMMON_H
#define COMMON_H

// --- Generic includes ---
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_check.h"
#include "esp_log.h"
#include "string.h"

// --- Project related includes ---


// --- Generic defines ---
#define LOW_PRIORITY 1
#define MED_PRIORITY 2
#define HIGH_PRIORITY 3
#define CRITICAL_PRIORITY 4
#define APP_MINIMAL_STACK_SIZE 2048

// --- Function definitions ---


#endif // COMMON_H
