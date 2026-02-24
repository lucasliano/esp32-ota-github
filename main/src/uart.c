// --------------- TODO: Ordenar Código -----------------
#include "uart.h"


void uart_print_line(const char *msg)
{
    if (!msg) {
        return;
    }
    uart_write_bytes(APP_UART_PORT, msg, strlen(msg));
    uart_write_bytes(APP_UART_PORT, "\r\n", 2);
}

esp_err_t app_uart_init(void)
{
    uart_config_t uart_cfg = {
        .baud_rate = APP_UART_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    uart_param_config(APP_UART_PORT, &uart_cfg);
    uart_set_pin(APP_UART_PORT, APP_UART_TX_PIN, APP_UART_RX_PIN,
                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(APP_UART_PORT, 1024, 0, 0, NULL, 0);
    uart_print_line("UART INIT");
    // NO cambiar esp_log_set_vprintf - los logs ESP_LOGI van a UART_NUM_0 (USB) automáticamente
    // esp_log_set_vprintf(uart_log_vprintf);  // Comentado para evitar conflictos

    return ESP_OK;
}
// ------------------------------------------------