/**
 * @file victron_crypto.h
 * @brief Módulo de desencriptado AES-128-CTR para protocolo Victron.
 *
 * Replica la lógica del repo victron-ble:
 * - Se valida el byte de verificación de clave.
 * - Se usa AES-128 en modo CTR con IV en little-endian.
 */

#ifndef VICTRON_CRYPTO_H
#define VICTRON_CRYPTO_H

#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"
#include "common.h"

// Tamaño de la clave AES (16 bytes = 128 bits)
#define VICTRON_AES_KEY_SIZE 16

// Device Key (AES key) en formato hex string
#ifdef MARAMBIO
    #define VICTRON_DEVICE_KEY_STR "b2d1c9a380d0a26381cf7d293333ed28"    // Marambio
#elifdef ISLA_VEGA
    #define VICTRON_DEVICE_KEY_STR "c61dc8fc18eee00aa0b8e213d8b6c780"    // Isla Vega
#else
    #define VICTRON_DEVICE_KEY_STR "b2d1c9a380d0a26381cf7d293333ed28"    // Marambio
#endif // MARAMBIO

/**
 * @brief Estructura para contexto de desencriptado
 */
typedef struct {
    uint8_t key[VICTRON_AES_KEY_SIZE];  // Clave AES
    uint32_t counter;                   // Contador para nonce
    uint16_t last_model_id;
    uint8_t last_readout_type;
    uint32_t last_iv;
} victron_crypto_ctx_t;

/**
 * @brief Inicializar contexto de desencriptado
 * @param ctx Contexto a inicializar
 * @param key_hex Clave en formato hex string (32 caracteres)
 * @return ESP_OK en éxito
 */
esp_err_t victron_crypto_init(victron_crypto_ctx_t *ctx, const char *key_hex);

/**
 * @brief Desencriptar payload usando AES-128-CTR.
 *
 * Similar a decrypt() en victron-ble/devices/base.py:
 * - Extrae IV del payload
 * - Verifica byte de clave
 * - Usa AES-CTR con counter little-endian
 *
 * @param ctx Contexto de desencriptado
 * @param encrypted Payload cifrado (contenedor Victron completo)
 * @param encrypted_len Longitud del payload
 * @param decrypted Buffer para datos desencriptados
 * @param decrypted_len Longitud del buffer de salida
 * @return ESP_OK en éxito, código de error en caso contrario
 */
esp_err_t victron_crypto_decrypt(
    victron_crypto_ctx_t *ctx,
    const uint8_t *encrypted,
    size_t encrypted_len,
    uint8_t *decrypted,
    size_t *decrypted_len
);

/**
 * @brief Obtener el header del último paquete desencriptado.
 */
void victron_crypto_get_last_header(const victron_crypto_ctx_t *ctx,
                                    uint16_t *model_id,
                                    uint8_t *readout_type,
                                    uint32_t *iv);

#endif // VICTRON_CRYPTO_H
