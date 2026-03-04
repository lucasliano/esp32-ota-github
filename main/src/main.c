/* Antartic project

   TODO: Complete docs

   Main application file. Initialization is done here. All tasks are created inside this file.
*/

#include "main.h"

static const char *TAG = "main_app";

 
TaskHandle_t led_task_Handle = NULL;
TaskHandle_t bme280_task_Handle = NULL;
TaskHandle_t victron_ble_task_Handle = NULL;
TaskHandle_t custom_metrics_task_Handle = NULL;


static void print_sha256(const uint8_t *image_hash, const char *label)
{
    char hash_print[HASH_LEN * 2 + 1];
    hash_print[HASH_LEN * 2] = 0;
    for (int i = 0; i < HASH_LEN; ++i) {
        sprintf(&hash_print[i * 2], "%02x", image_hash[i]);
    }
    ESP_LOGI(TAG, "%s %s", label, hash_print);
}

static void get_sha256_of_partitions(void)
{
    uint8_t sha_256[HASH_LEN] = { 0 };
    esp_partition_t partition;

    // get sha256 digest for bootloader
    partition.address   = ESP_BOOTLOADER_OFFSET;
    partition.size      = ESP_PARTITION_TABLE_OFFSET;
    partition.type      = ESP_PARTITION_TYPE_APP;
    esp_partition_get_sha256(&partition, sha_256);
    print_sha256(sha_256, "SHA-256 for bootloader: ");

    // get sha256 digest for running partition
    esp_partition_get_sha256(esp_ota_get_running_partition(), sha_256);
    print_sha256(sha_256, "SHA-256 for current firmware: ");
}

esp_err_t nvs_init(void)
{
    /**
     * @brief Initialize NVS memory.
     *
     * @param None  None.
     * @return esp_err_t
     *         - ESP_OK on success
     *         - Should reboot on error. Otherwise returns ESP_FAIL.
     */
    // Initialize NVS.
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // 1.OTA app partition table has a smaller NVS partition size than the non-OTA
        // partition table. This size mismatch may cause NVS initialization to fail.
        // 2.NVS partition contains data in new format and cannot be recognized by this version of code.
        // If this happens, we erase NVS partition and initialize NVS again.
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    
    if(err != ESP_OK)
    {
        ESP_LOGE(TAG, "Fatal: NVS init failed. Rebooting.");
        ESP_ERROR_CHECK(err);
    }

    return err;
}

static const char *reset_reason_str(esp_reset_reason_t r)
{
    switch (r) {
        case ESP_RST_UNKNOWN:   return "UNKNOWN";
        case ESP_RST_POWERON:   return "POWERON";
        case ESP_RST_EXT:       return "EXTERNAL (EN pin)";
        case ESP_RST_SW:        return "SOFTWARE (esp_restart)";
        case ESP_RST_PANIC:     return "PANIC/EXCEPTION";
        case ESP_RST_INT_WDT:   return "INT WDT";
        case ESP_RST_TASK_WDT:  return "TASK WDT";
        case ESP_RST_WDT:       return "OTHER WDT";
        case ESP_RST_BROWNOUT:  return "BROWNOUT";
        case ESP_RST_CPU_LOCKUP:return "CPU LOCKUP";
        case ESP_RST_PWR_GLITCH:return "POWER GLITCH";
        default:                return "OTHER";
    }
}

esp_err_t system_init(void)
{
    ESP_RETURN_ON_ERROR(app_uart_init(), TAG, "app_uart_init() failed.");
    ESP_RETURN_ON_ERROR(log_mux_init(), TAG, "Log multiplexer failed");
    ESP_RETURN_ON_ERROR(nvs_flash_init(), TAG, "NVS init failed");
    get_sha256_of_partitions();
    ESP_RETURN_ON_ERROR(esp_event_loop_create_default(), TAG, "esp_event_loop_create_default() failed.");
    ESP_RETURN_ON_ERROR(gpio_init(), TAG, "GPIO init failed");
    ESP_RETURN_ON_ERROR(i2c_master_init(), TAG, "I2C init failed");
    ESP_RETURN_ON_ERROR(bme280_init(), TAG, "BME280 was not detected on BME280_I2C_ADDR");
    ESP_RETURN_ON_ERROR(led_task_init(), TAG, "led_task_init() failed");
    ESP_RETURN_ON_ERROR(wifi_init(), TAG, "WiFi init failed");
    
    // ESP_RETURN_ON_ERROR(rtc_sntp_init(), TAG, "SNTP init failed");
    // syslog_udp_start();

    return ESP_OK;
}

esp_err_t system_health_checkup(void)
{
    /**
     * @brief Runs a health check and then validates the operation of a new ota image.
     *
     * @param None  None.
     * @return esp_err_t
     *         - ESP_OK on success
     *         - Otherwise returns ESP_FAIL.
     */
    // esp_err_t err = ESP_FAIL;

    // System health check goes here:

    esp_reset_reason_t r = esp_reset_reason();
    
    // If an anormal reset took place, run OTA first to search for a new image.
    if ((r != ESP_RST_SW) && (r != ESP_RST_POWERON)) 
    {
        ESP_LOGW(TAG, "Reset reason: %s (%d)", reset_reason_str(r), (int)r);

        ESP_RETURN_ON_ERROR(nvs_flash_init(), TAG, "NVS init failed");
        get_sha256_of_partitions();
        ESP_RETURN_ON_ERROR(esp_event_loop_create_default(), TAG, "esp_event_loop_create_default() failed.");
        ESP_RETURN_ON_ERROR(gpio_init(), TAG, "GPIO init failed");
        ESP_RETURN_ON_ERROR(wifi_init(), TAG, "WiFi init failed");
        ESP_LOGI(TAG, "Running OTA..");
        if (wifi_connect_if_needed() == ESP_FAIL) 
        {
            ESP_LOGE(TAG, "wifi_connect_if_needed() error on relay_task");
            esp_restart();
        }
        execute_ota();
        // Will reboot after OTA
    }

    return system_init();
}

void app_main(void)
{
    ESP_LOGI(TAG, "Applitacion: ##### OTA #####");
    
    // Check if everthing is working. Runs rollback if not.
    if (system_health_checkup() == ESP_OK)
    {
        ESP_LOGI(TAG, "System health check passed.");
        if (esp_ota_mark_app_valid_cancel_rollback() != ESP_OK){
            ESP_LOGE(TAG, "Error occured during esp_ota_mark_app_valid_cancel_rollback(). Rebooting.."); 
            esp_restart();
        } 

    }else{
        ESP_LOGE(TAG, "Error occured during system health check.");
        ESP_LOGE(TAG, "Running Rollback.");
        esp_ota_mark_app_invalid_rollback_and_reboot(); // App is not working fine. Rollback + reboot.
        esp_restart(); // If it fails, force reboot. It usually fails because theres no valid image in slot.
    }


    // NOTE: Para testear un PANIC y ver si busca update primero.
    // esp_system_abort("PANIC TEST: provocado a proposito");


    // xTaskCreate(&adjust_time_task, "adjust_time_task", 2*APP_MINIMAL_STACK_SIZE , NULL, LOW_PRIORITY, NULL);
    xTaskCreate(&led_task, "led_task", 2*APP_MINIMAL_STACK_SIZE , NULL, LOW_PRIORITY, &led_task_Handle);
    xTaskCreate(&bme280_task, "bme280_task", 2*APP_MINIMAL_STACK_SIZE, NULL, MED_PRIORITY, &bme280_task_Handle);
    xTaskCreate(&victron_ble_task, "victron_ble_task", 6*APP_MINIMAL_STACK_SIZE, NULL, MED_PRIORITY, &victron_ble_task_Handle);
    xTaskCreate(&custom_metrics_task, "metrics_task", 2*APP_MINIMAL_STACK_SIZE, NULL, MED_PRIORITY, &custom_metrics_task_Handle);
    
    xTaskCreate(&relay_task, "relay_task", 2*APP_MINIMAL_STACK_SIZE , NULL, CRITICAL_PRIORITY, NULL);

}