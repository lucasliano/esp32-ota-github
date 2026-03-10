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

#define OTA_RETRY_TIMEOUT_MS    30*1000


#ifdef MARAMBIO
#define CONFIG_EXAMPLE_FIRMWARE_UPGRADE_URL "https://raw.githubusercontent.com/lucasliano/esp32-ota-github/main/public_images/marambio.bin"
#elifdef ISLA_VEGA
#define CONFIG_EXAMPLE_FIRMWARE_UPGRADE_URL "https://raw.githubusercontent.com/lucasliano/esp32-ota-github/main/public_images/isla_vega.bin"
#else
#define CONFIG_EXAMPLE_FIRMWARE_UPGRADE_URL "https://raw.githubusercontent.com/lucasliano/esp32-ota-github/dev/public_images/test.bin"
#endif // MARAMBIO




void execute_ota(void);
esp_err_t _http_event_handler(esp_http_client_event_t *evt);

#endif // OTA_H