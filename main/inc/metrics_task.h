/**
 * @file metrics_task.h
 * @brief Envío de todas las métricas (Victron, BME280, WiFi, relay, system) a InfluxDB para Grafana.
 */

#ifndef METRICS_TASK_H
#define METRICS_TASK_H

#include "esp_err.h"
#include "common.h"

/* InfluxDB: misma URL/org/bucket que en bme280_task (Grafana usa Influx como datasource) */
// #define METRICS_INFLUX_URL   "http://172.16.0.247:8086"
#define METRICS_INFLUX_URL   "http://influxdb.giar.com.ar:80"
#define METRICS_INFLUX_TOKEN "ssvoFRm8Gl0iTw1dqMUoOyf6XmKpfQvMv6d16cFVCb7qG98EVrtt-PmmQnUHew0QgGPwE-PiLAy7R4j5V5PPXw=="
#define METRICS_INFLUX_ORG   "lab"
#define METRICS_INFLUX_BUCKET "esp"


#ifdef MARAMBIO
    #define LOCATION_TAG "MARAMBIO"    // Marambio
#elifdef ISLA_VEGA
    #define LOCATION_TAG "ISLAVEGA"    // Isla Vega
#else
    #define LOCATION_TAG "UNKNOWN"     // Test
#endif // MARAMBIO


esp_err_t mppt_send_to_influx(
    int charge_state, 
    int charger_error,
    float batt_v,
    float batt_a,
    float yield_wh,
    float solar_w,
    float ext_load
);
esp_err_t bme_send_to_influx(float temp_c, float hum_pct, float press_hpa);

esp_err_t system_send_to_influx(void);
void custom_metrics_task(void *arg);





// ==========================================



/* Intervalo de envío a InfluxDB (segundos) */
#define METRICS_INTERVAL_SEC 1

/** Actualizar última lectura BME280 (temp °C, humedad %, presión hPa) */
void metrics_set_bme280(float temp_c, float hum_pct, float press_hpa);

/** Actualizar últimos datos Victron MPPT (llamar desde ble_task cuando llega payload) */
void metrics_set_victron(int charge_state, int charger_error,
                        float batt_v, float batt_a, float yield_wh, float solar_w, float ext_load);

/** Actualizar resultado del ping (tx, rx, time_ms). Llamar desde wifi_task ping_on_end */
void metrics_set_wifi_ping(unsigned tx, unsigned rx, unsigned time_ms);

/** Actualizar RSSI WiFi (dBm). Llamar desde wifi_task al conectar */
void metrics_set_wifi_rssi(int8_t rssi);

/** Actualizar estado del relay (1=ON, 0=OFF). Llamar desde relay_task */
void metrics_set_relay(int state);

/** Iniciar tarea que envía métricas a InfluxDB cada METRICS_INTERVAL_SEC */
esp_err_t metrics_task_start(void);

void metrics_task(void *arg);


#endif /* METRICS_TASK_H */
