/**
 * @file gpio.h
 * @brief Definition of GPIO related functions.
 */

#ifndef GPIO_H
#define GPIO_H

// --- Generic includes ---
#include "common.h"
#include "driver/gpio.h"
#include "wifi.h"
#include "ota.h"


// --- Project related includes ---


// --- Generic defines ---
#define RELAY_GPIO_NUM GPIO_NUM_26
#define RELAY_INITIAL_CONDICION 1     // Starlink ON
#define RELAY_ON_MS  (15 * 60 * 1000) // 15m prendido 
#define RELAY_OFF_MS (85500 * 1000)   // 24h-15m apagado 
// #define RELAY_ON_MS  (5 * 60 * 1000) // 5m prendido 
// #define RELAY_OFF_MS (2 * 60 * 1000)   // 2m apagado 


// --- Function definitions ---
esp_err_t gpio_init(void);
void relay_task(void *pvParameter);


#endif // GPIO_H