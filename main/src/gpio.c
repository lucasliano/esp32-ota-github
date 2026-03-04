#include "gpio.h"

static const char *TAG = "RELAY_TASK";

extern TaskHandle_t led_task_Handle;
extern TaskHandle_t bme280_task_Handle;
extern TaskHandle_t victron_ble_task_Handle;
extern TaskHandle_t custom_metrics_task_Handle;

esp_err_t gpio_init(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << RELAY_GPIO_NUM,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    gpio_set_level(RELAY_GPIO_NUM, RELAY_INITIAL_CONDICION);

    return ESP_OK;
}


void relay_task(void *pvParameter)
{
    while (1) {
        // Turns on power and then turns on wifi
        gpio_set_level(RELAY_GPIO_NUM, 1);
        ESP_LOGI(TAG, "Relay ON");

        if (wifi_connect_if_needed() == ESP_FAIL) ESP_LOGE(TAG, "wifi_connect_if_needed() error on relay_task");
        vTaskDelay(pdMS_TO_TICKS(RELAY_ON_MS));

        // Turns off wifi before cutting power
        if (wifi_disconnect_if_connected() == ESP_FAIL) ESP_LOGE(TAG, "wifi_disconnect_if_connected() error on relay_task");
        gpio_set_level(RELAY_GPIO_NUM, 0);
        ESP_LOGI(TAG, "Relay OFF");
        vTaskDelay(pdMS_TO_TICKS(RELAY_OFF_MS));

        gpio_set_level(RELAY_GPIO_NUM, 1);
        // vTaskDelay(pdMS_TO_TICKS(120*1000));    // Delay to let starlink initialize
        vTaskDelay(pdMS_TO_TICKS(3*1000));    // Delay to let starlink initialize


        // Stops all tasks to avoid conflicts while running OTA
        vTaskDelete(led_task_Handle);
        vTaskDelete(bme280_task_Handle);
        vTaskDelete(victron_ble_task_Handle);
        vTaskDelete(custom_metrics_task_Handle);
        vTaskDelay(pdMS_TO_TICKS(10));    // Delay to let starlink initialize


        ESP_LOGI(TAG, "Running OTA..");
        if (wifi_connect_if_needed() == ESP_FAIL) 
        {
            ESP_LOGE(TAG, "wifi_connect_if_needed() error on relay_task");
            esp_restart();
        }
        execute_ota();
    }
}

