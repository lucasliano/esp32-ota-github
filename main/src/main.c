/* Antartic project

   TODO: Complete docs

   Main application file. Initialization is done here. All tasks are created inside this file.
*/

#include "main.h"

#define HASH_LEN 32


static const char *TAG = "main_app";


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
    esp_err_t err = ESP_FAIL;

    // System health check goes here:
    // TODO: Complete

    err = ESP_OK;
    return err;
}

esp_err_t system_init(void)
{

    ESP_RETURN_ON_ERROR(nvs_flash_init(), TAG, "NVS init failed");
    get_sha256_of_partitions();
    ESP_RETURN_ON_ERROR(app_uart_init(), TAG, "app_uart_init() failed.");
    ESP_RETURN_ON_ERROR(esp_event_loop_create_default(), TAG, "esp_event_loop_create_default() failed.");
    ESP_RETURN_ON_ERROR(gpio_init(), TAG, "GPIO init failed");
    ESP_RETURN_ON_ERROR(led_task_init(), TAG, "led_task_init() failed");
    ESP_RETURN_ON_ERROR(wifi_init(), TAG, "WiFi init failed");
    // ESP_RETURN_ON_ERROR(rtc_sntp_init(), TAG, "SNTP init failed");
    
    // TODO: Remove all uart_print_line() in the project. Instead add some function to send los to uart.
    uart_print_line("init ok\n");

    return ESP_OK;
}


void app_main(void)
{
    ESP_LOGI(TAG, "Applitacion: ##### OTA #####");
    
    // Check if everthing is working. Runs rollback if not.
    if (system_health_checkup() == ESP_OK)
    {
        ESP_LOGI(TAG, "System health check passed.");
        esp_ota_mark_app_valid_cancel_rollback();   // App works fine.
    }else{
        ESP_LOGE(TAG, "Error occured during system health check.");
        ESP_LOGE(TAG, "Running Rollback.");
        esp_ota_mark_app_invalid_rollback_and_reboot(); // App is not working fine. Rollback + reboot.
    }
    
    
    ESP_ERROR_CHECK(system_init()); // Reboots on error


    // xTaskCreate( TaskFunction_t pxTaskCode,
    //              const char * const pcName, /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
    //              const configSTACK_DEPTH_TYPE usStackDepth,
    //              void * const pvParameters,
    //              UBaseType_t uxPriority,
    //              TaskHandle_t * const pxCreatedTask )



    xTaskCreate(&relay_task, "relay_task", 2*APP_MINIMAL_STACK_SIZE , NULL, CRITICAL_PRIORITY, NULL);
    // xTaskCreate(&adjust_time_task, "adjust_time_task", 2*APP_MINIMAL_STACK_SIZE , NULL, LOW_PRIORITY, NULL);
    xTaskCreate(&led_task, "led_task", 2*APP_MINIMAL_STACK_SIZE , NULL, LOW_PRIORITY, NULL);

}