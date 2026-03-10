/**
 * @file udp_client.c
 * @brief File with UDP client related functions and task.
 */
#include "udp_client.h"
#include "uart.h"

// -------------------- Globals --------------------
static RingbufHandle_t s_rb = NULL;
static const char *TAG = "udp_client.C";


// -------------------- Buffers --------------------
void syslog_enqueue(const char *msg, int len)
{
    BaseType_t  err;
    if (!s_rb || !msg || len <= 0) return;

    // Ringbuffer copies the data, so stack buffer is OK
    err = xRingbufferSend(s_rb, (void *)msg, len, 0);

    if (err == pdFALSE)
    {
        uart_write_bytes(UART_NUM_0, "\r\n\r\n BUFFER ERROR ", 20);
        uart_write_bytes(UART_NUM_0, msg, len);
        uart_write_bytes(UART_NUM_0, "\r", 2);
    }
}


// -------------------- Helpers --------------------
static int get_timestamp(char *out, size_t out_sz)
{
    // out should be a char[28] to work properly

    time_t now = time(NULL);
    if (now == (time_t)-1) return -1;

    struct tm tm_local;
    if (localtime_r(&now, &tm_local) == NULL) return -1;

    // "YYYY-MM-DD HH-MM-SS"
    size_t n = strftime(out, out_sz, "%Y-%m-%d %H-%M-%S", &tm_local);
    if (n == 0) return -1;

    // Agregar el sufijo exacto pedido
    int w = snprintf(out + n, out_sz - n, " (UTC-3)");
    if (w < 0 || (size_t)w >= out_sz - n) return -1;

    return 0;
}


// -------------------- UDP sender task --------------------
static void syslog_task(void *arg)
{
    (void)arg;

    // Resolve host once (simple approach). You can re-resolve on error if needed.
    struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_DGRAM,
    };
    struct addrinfo *res = NULL;

    char port_str[8];
    snprintf(port_str, sizeof(port_str), "%d", UDP_SERVER_PORT);

    
    for (;;) {
        if (getaddrinfo(UDP_SERVER_IP, port_str, &hints, &res) != 0 || !res) 
        {
            freeaddrinfo(res);
            vTaskDelay(pdMS_TO_TICKS(RETRY_TIMEOUT));
            return;
        }

        // Create socket
        int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sock < 0)
        {
            ESP_LOGE(TAG, "Error while trying to create socket: errno=%d (%s)", errno, strerror(errno));
            close(sock);
            freeaddrinfo(res);
            vTaskDelay(pdMS_TO_TICKS(RETRY_TIMEOUT));
            continue;
        }


        size_t item_size = 0;
        char *item = (char *) xRingbufferReceive(s_rb, &item_size, pdMS_TO_TICKS(1000));
        if (!item)
        {
            close(sock);
            freeaddrinfo(res);
            vTaskDelay(pdMS_TO_TICKS(RETRY_TIMEOUT));
            continue;
        }

        // Create Pprefix with timestamp
        char timestamp[28];
        const char *sep = " | ";
        if (get_timestamp(timestamp, sizeof(timestamp)) != 0) 
        {
            ESP_LOGE(TAG, "get_timestamp() failed");
            // vRingbufferReturnItem(s_rb, item);
            close(sock);
            freeaddrinfo(res);
            vTaskDelay(pdMS_TO_TICKS(RETRY_TIMEOUT));
            continue;
        }

        // Allocate buffer memory: timestamp + sep + item
        size_t timestamp_len  = strlen(timestamp);
        size_t sep_len = strlen(sep);
        size_t total = timestamp_len + sep_len + item_size;
        
        // Copy item to local memory and add prefix
        char *copy = (char *)malloc(total+1);
        if (copy == NULL) 
        {
            ESP_LOGE(TAG, "Cannot allocate memory for local buffer");
            // vRingbufferReturnItem(s_rb, item);
            close(sock);
            freeaddrinfo(res);
            vTaskDelay(pdMS_TO_TICKS(RETRY_TIMEOUT));
            continue;
        }
        memcpy(copy, timestamp, timestamp_len);
        memcpy(copy + timestamp_len, sep, sep_len);
        memcpy(copy + timestamp_len + sep_len, item, item_size);
        copy[total] = '\0';

        // Send over UDP socket
        ssize_t sent = sendto(sock, copy, total, 0, res->ai_addr, res->ai_addrlen);
        if (sent < 0) {
            ESP_LOGE(TAG, "sendto failed: errno=%d (%s)", errno, strerror(errno));
            ESP_LOGE(TAG, "total size =%d", total);
            free(copy);
            close(sock);
            freeaddrinfo(res);
            vTaskDelay(pdMS_TO_TICKS(RETRY_TIMEOUT));
            continue;
        } else if ((size_t)sent != total) {
            // En UDP normalmente es todo o error, pero lo dejamos defensivo
            ESP_LOGW(TAG, "sendto envió %d/%u bytes", (int)sent, (unsigned)total);
        }

        // Free ring buffer memory
        vRingbufferReturnItem(s_rb, (void *)item);

        // Free local buffer
        free(copy);

        // Close socket
        close(sock);

        freeaddrinfo(res);
    }

    // not reached
    // close(sock); freeaddrinfo(res);
}

void syslog_udp_start(void)
{
    if (!s_rb) {
        s_rb = xRingbufferCreate(CIRCULAR_BUFFER_SIZE, RINGBUF_TYPE_NOSPLIT);
    }
    xTaskCreate(syslog_task, "syslog_task", 2*APP_MINIMAL_STACK_SIZE, NULL, LOW_PRIORITY, NULL);
}