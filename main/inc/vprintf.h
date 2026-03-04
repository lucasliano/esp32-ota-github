/**
 * @file VPRINTF.h
 * @brief Generic definitions among various .h files.
 */

#ifndef VPRINTF_H
#define VPRINTF_H

// --- Generic includes ---
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "uart.h"
#include "udp_client.h"

// --- Generic defines ---


// --- Function definitions ---
esp_err_t log_mux_init(void);
static int custom_vprintf(const char *fmt, va_list ap);

#endif // VPRINTF_H
