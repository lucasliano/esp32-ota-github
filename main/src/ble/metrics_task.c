/**
 * @file metrics_task.c
 * @brief Agrega métricas de todos los módulos y las envía a InfluxDB para Grafana.
 */

#include "metrics_task.h"

#include <string.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_mac.h"
#include "wifi.h"

static const char *TAG = "metrics_task.c";
static const char *LOC = LOCATION_TAG;

static uint32_t system_metric_error_counter = 0;

static int get_mac_str(char * buf)
{
    // Read MAC
    uint8_t mac[6];
    if (esp_read_mac(mac, ESP_MAC_WIFI_STA) != ESP_OK){
        ESP_LOGE(TAG, "esp_read_mac() failed");
        return ESP_FAIL;
    }
    return snprintf(buf, 18, "%02X:%02X:%02X:%02X:%02X:%02X",mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

static esp_err_t send_metrics(char* body, size_t body_len)
{
    if (!wifi_is_connected()){
        return ESP_FAIL;
    }

    char url[256];
    snprintf(url, sizeof(url), "%s/api/v2/write?org=%s&bucket=%s&precision=s",
             METRICS_INFLUX_URL, METRICS_INFLUX_ORG, METRICS_INFLUX_BUCKET);

    char auth_header[128];
    snprintf(auth_header, sizeof(auth_header), "Token %s", METRICS_INFLUX_TOKEN);

    esp_http_client_config_t cfg = {
        .url = url,
        .method = HTTP_METHOD_POST,
        .timeout_ms = 10000,
    };
    esp_http_client_handle_t client = esp_http_client_init(&cfg);
    if (!client) {
        ESP_LOGE(TAG, "Influx - HTTP client init fallo");
        return ESP_FAIL;
    }
    esp_http_client_set_header(client, "Authorization", auth_header);
    esp_http_client_set_header(client, "Content-Type", "text/plain; charset=utf-8");
    esp_http_client_set_post_field(client, body, (int)body_len);

    esp_err_t err = esp_http_client_perform(client);
    if (err != ESP_OK) 
        ESP_LOGE(TAG, "Influx POST err 0x%x", err);

    esp_http_client_cleanup(client);
    return err;
}

esp_err_t mppt_send_to_influx(int charge_state, int charger_error,
                        float batt_v, float batt_a, float yield_wh, float solar_w, float load_a)
{
    char mac_str[18];
    get_mac_str(mac_str);

    // InfluxdB Query definition
    const char *fmt =
        "victron,mac=%s,loc=%s charge_state=%di,charger_error=%di,"
        "batt_v=%.2f,batt_a=%.2f,yield_wh=%.1f,solar_w=%.1f,load_a=%.2f\n";

    // Calculates the size of the buffer needed
    int needed = snprintf(NULL, 0, fmt,
                          mac_str, LOC, charge_state, charger_error,
                          batt_v, batt_a, yield_wh, solar_w, load_a);

    if (needed < 0) {
        ESP_LOGE(TAG, "snprintf size calc failed");
        return ESP_FAIL;
    }

    // Creates buffer with from heap to avoid stack overflow
    size_t buf_sz = (size_t)needed + 1;   // +1 for '\0'
    char *body = malloc(buf_sz);
    if (!body) {
        ESP_LOGE(TAG, "malloc(%u) failed", (unsigned)buf_sz);
        return ESP_ERR_NO_MEM;
    }

    // Copies query to buffer
    int written = snprintf(body, buf_sz, fmt,
                           mac_str, LOC, charge_state, charger_error,
                           batt_v, batt_a, yield_wh, solar_w, load_a);

    if (written < 0 || (size_t)written >= buf_sz) {
        ESP_LOGE(TAG, "snprintf write failed/truncated");
        free(body);
        return ESP_FAIL;
    }

    // Sends query to influx
    esp_err_t err = send_metrics(body, (size_t)written);
    if (err == ESP_OK) 
        ESP_LOGI(TAG, "MPPT metrics were sent to InfluxDB (%u bytes)", (unsigned) written);
    
    // Free heap to avoid memory leakage
    free(body);
    return err;
}


esp_err_t bme_send_to_influx(float temp_c, float hum_pct, float press_hpa)
{
    char mac_str[18];
    get_mac_str(mac_str);

    // InfluxdB Query definition
    const char *fmt = "bme280,mac=%s,loc=%s temp_c=%.2f,hum=%.2f,press_hpa=%.2f\n";

    // Calculates the size of the buffer needed
    int needed = snprintf(NULL, 0, fmt, mac_str, LOC, temp_c, hum_pct, press_hpa);

    if (needed < 0) {
        ESP_LOGE(TAG, "snprintf size calc failed");
        return ESP_FAIL;
    }

    // Creates buffer with from heap to avoid stack overflow
    size_t buf_sz = (size_t)needed + 1;   // +1 for '\0'
    char *body = malloc(buf_sz);
    if (!body) {
        ESP_LOGE(TAG, "malloc(%u) failed", (unsigned)buf_sz);
        return ESP_ERR_NO_MEM;
    }

    // Copies query to buffer
    int written = snprintf(body, buf_sz, fmt, mac_str, LOC, temp_c, hum_pct, press_hpa);

    if (written < 0 || (size_t)written >= buf_sz) {
        ESP_LOGE(TAG, "snprintf write failed/truncated");
        free(body);
        return ESP_FAIL;
    }

    // Sends query to influx
    esp_err_t err = send_metrics(body, (size_t)written);
    if (err == ESP_OK) 
        ESP_LOGI(TAG, "BME metrics were sent to InfluxDB (%u bytes)", (unsigned) written);
    
    // Free heap to avoid memory leakage
    free(body);
    return err;
}

esp_err_t system_send_to_influx(void)
{
    char mac_str[18];
    get_mac_str(mac_str);

    // Calculates system parameter
    uint32_t heap_size = esp_get_free_heap_size();
    int64_t uptime_sec = esp_timer_get_time() / 1000000;

    // InfluxdB Query definition
    const char *fmt = "system,mac=%s,loc=%s heap_free=%lu,uptime_sec=%lld,sys_error_counter=%lu\n";

    // Calculates the size of the buffer needed
    int needed = snprintf(NULL, 0, fmt, mac_str, LOC, heap_size, uptime_sec,system_metric_error_counter);

    if (needed < 0) {
        ESP_LOGE(TAG, "snprintf size calc failed");
        return ESP_FAIL;
    }

    // Creates buffer with from heap to avoid stack overflow
    size_t buf_sz = (size_t)needed + 1;   // +1 for '\0'
    char *body = malloc(buf_sz);
    if (!body) {
        ESP_LOGE(TAG, "malloc(%u) failed", (unsigned)buf_sz);
        return ESP_ERR_NO_MEM;
    }

    // Copies query to buffer
    int written = snprintf(body, buf_sz, fmt, mac_str, LOC, heap_size, uptime_sec,system_metric_error_counter);

    if (written < 0 || (size_t)written >= buf_sz) {
        ESP_LOGE(TAG, "snprintf write failed/truncated");
        free(body);
        return ESP_FAIL;
    }

    // Sends query to influx
    esp_err_t err = send_metrics(body, (size_t)written);
    if (err == ESP_OK){
        system_metric_error_counter = 0;
        ESP_LOGI(TAG, "System metrics were sent to InfluxDB (%u bytes)", (unsigned) written);
    }else{
        if (system_metric_error_counter != UINT32_MAX) {
            system_metric_error_counter++;
        }
    }
    
    // Free heap to avoid memory leakage
    free(body);
    return err;
}


void custom_metrics_task(void *arg)
{
    (void)arg;
    TickType_t last = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(METRICS_INTERVAL_SEC * 1000); // 100ms

    while (1) {
        if(system_send_to_influx() != ESP_OK)
        {
            ESP_LOGE(TAG, "An error occured while trying to send system metrics");
        }
        vTaskDelayUntil(&last, period);
    }
}