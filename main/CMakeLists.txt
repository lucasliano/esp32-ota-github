set(SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/src")
set(SRCS
    "main.c"
    "ota.c"
    "wifi.c"
    "gpio.c"
    "rtc.c"
    "led_task.c"
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
        
    
    # Embed the server root certificate into the final binary
    # NOTE: Not needed if using bundle with github
    # EMBED_TXTFILES ${project_dir}/server_certs/ca_cert.pem
)
