
/**
 * @file UART.h
 * @brief Tarea FreeRTOS para controlar LED onboard.
 */

#ifndef UART_H
#define UART_H


// --- Generic includes ---
#include "common.h"
#include "driver/uart.h"
#include "gpio.h"


// --- Project related includes ---



// --- Generic defines ---
#define APP_UART_PORT UART_NUM_2
#define APP_UART_TX_PIN GPIO_NUM_17
#define APP_UART_RX_PIN GPIO_NUM_16
#define APP_UART_BAUD   115200


// --- Function definitions ---
esp_err_t app_uart_init(void);
void uart_print_line(const char *msg);



#endif // UART_H
