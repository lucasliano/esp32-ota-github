set(SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/src")
set(SRCS
    "main.c"
    "ota.c"
    "wifi.c"
    "gpio.c"
    "rtc.c"
    "led_task.c"
    "uart.c"
    "wtd.c"
    "vprintf.c"
    "udp_client.c"
    "ble/ble_task.c"
    "ble/victron_ble.c"
    "ble/victron_crypto.c"
    "ble/victron_parser.c"
    "ble/metrics_task.c"
    "bme/bme280.c"
)
list(TRANSFORM SRCS PREPEND "${SRC_DIR}/")


idf_build_get_property(project_dir PROJECT_DIR)
idf_component_register(
    SRCS ${SRCS}
    INCLUDE_DIRS "inc/"
    REQUIRES 
        esp_http_client 
        app_update
        esp_https_ota
        mbedtls
        nvs_flash
        esp_psram
        driver
        bt
        lwip
        esp_driver_gpio
        esp_driver_uart
        esp_driver_i2c
        esp_timer
        esp_wifi
        esp_netif
        esp_event
        protocol_examples_common
)