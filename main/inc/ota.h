/**
 * @file ota.h
 * @brief Definition of all Over-The-Air related functions.
 */

#ifndef OTA_H
#define OTA_H

// --- Generic includes ---
#include "common.h"
#include "esp_ota_ops.h"
#include "esp_https_ota.h"
#include "esp_http_client.h"
#ifdef CONFIG_EXAMPLE_USE_CERT_BUNDLE
#include "esp_crt_bundle.h"
#endif

#include "uart.h"
#include "wtd.h"




void execute_ota(void);
esp_err_t _http_event_handler(esp_http_client_event_t *evt);

#endif // OTA_H