#include "gpio.h"

static const char *TAG = "RELAY_TASK";

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




        if (wifi_connect_if_needed() == ESP_FAIL) ESP_LOGE(TAG, "wifi_connect_if_needed() error on relay_task");
        execute_ota();
    }
}

