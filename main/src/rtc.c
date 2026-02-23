#include "rtc.h"


/* LwIP SNTP example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

static const char *TAG = "rtc.c";

#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 48
#endif


void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}

/**
 * Formatea la hora local actual en un buffer provisto por el usuario.
 * Retorna puntero al mismo buffer (o NULL si falla).
 */
char *time_now_str(char *buf, size_t buf_len)
{
    if (buf == NULL || buf_len == 0) {
        return NULL;
    }

    time_t now;
    struct tm timeinfo;

    time(&now);

    if (localtime_r(&now, &timeinfo) == NULL) {
        // En caso de error, devuelve string vacío
        if (buf_len > 0) {
            buf[0] = '\0';
        }
        return NULL;
    }

    // Formato más estable que %c (evita depender de locale)
    // Ejemplo: 2026-02-22 14:35:10
    if (strftime(buf, buf_len, "%Y-%m-%d %H:%M:%S", &timeinfo) == 0) {
        // Buffer insuficiente
        if (buf_len > 0) {
            buf[0] = '\0';
        }
        return NULL;
    }

    return buf;
}

const char *get_current_time(void)
{
    static char buf[32];

    if (time_now_str(buf, sizeof(buf)) == NULL) {
        return "TIME_ERROR";
    }

    return buf;
}

esp_err_t rtc_sntp_init(void)
{
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    // Is time set? If not, tm_year will be (1970 - 1900).
    if (timeinfo.tm_year < (2016 - 1900)) {
        ESP_LOGI(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
        obtain_time();
        // update 'now' variable with current time
        time(&now);
    }
    setenv("TZ", "UTC+3", 1);
    tzset();

    ESP_LOGI(TAG, "Hora local (UTC-3): %s\n", get_current_time());

    return ESP_OK;
}


void adjust_time_task(void *pvParameter)
{
    struct timeval outdelta;

    while(1)
    {
        ESP_LOGI(TAG, "Synchronizing time..");
        if (sntp_get_sync_mode() == SNTP_SYNC_MODE_SMOOTH) {
            while (sntp_get_sync_status() == SNTP_SYNC_STATUS_IN_PROGRESS) {
                adjtime(NULL, &outdelta);
                vTaskDelay(2000 / portTICK_PERIOD_MS);
            }
        }

        ESP_LOGI(TAG, "Time was sync");
        ESP_LOGW(TAG, "Hora local (UTC-3): %s\n", get_current_time());
        vTaskDelay(pdMS_TO_TICKS(TIME_UPDATE_MS));
    }
}


static void print_servers(void)
{
    ESP_LOGI(TAG, "List of configured NTP servers:");

    for (uint8_t i = 0; i < SNTP_MAX_SERVERS; ++i){
        if (esp_sntp_getservername(i)){
            ESP_LOGI(TAG, "server %d: %s", i, esp_sntp_getservername(i));
        } else {
            // we have either IPv4 or IPv6 address, let's print it
            char buff[INET6_ADDRSTRLEN];
            ip_addr_t const *ip = esp_sntp_getserver(i);
            if (ipaddr_ntoa_r(ip, buff, INET6_ADDRSTRLEN) != NULL)
                ESP_LOGI(TAG, "server %d: %s", i, buff);
        }
    }
}

static void obtain_time(void)
{
#if LWIP_DHCP_GET_NTP_SRV
    /**
     * NTP server address could be acquired via DHCP,
     * see following menuconfig options:
     * 'LWIP_DHCP_GET_NTP_SRV' - enable STNP over DHCP
     * 'LWIP_SNTP_DEBUG' - enable debugging messages
     *
     * NOTE: This call should be made BEFORE esp acquires IP address from DHCP,
     * otherwise NTP option would be rejected by default.
     */
    ESP_LOGI(TAG, "Initializing SNTP");
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG(CONFIG_SNTP_TIME_SERVER);
    config.start = false;                       // start SNTP service explicitly (after connecting)
    config.server_from_dhcp = true;             // accept NTP offers from DHCP server, if any (need to enable *before* connecting)
    config.renew_servers_after_new_IP = true;   // let esp-netif update configured SNTP server(s) after receiving DHCP lease
    config.index_of_first_server = 1;           // updates from server num 1, leaving server 0 (from DHCP) intact
    // configure the event on which we renew servers
#ifdef CONFIG_EXAMPLE_CONNECT_WIFI
    config.ip_event_to_renew = IP_EVENT_STA_GOT_IP;
#else
    config.ip_event_to_renew = IP_EVENT_ETH_GOT_IP;
#endif
    config.sync_cb = time_sync_notification_cb; // only if we need the notification function
    esp_netif_sntp_init(&config);

#endif /* LWIP_DHCP_GET_NTP_SRV */



    ESP_LOGI(TAG, "Starting SNTP");
    esp_netif_sntp_start();
    print_servers();

    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 15;
    while (esp_netif_sntp_sync_wait(2000 / portTICK_PERIOD_MS) == ESP_ERR_TIMEOUT && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
    }
    time(&now);
    localtime_r(&now, &timeinfo);

    // TODO: Agregar esto en el deinit
    // esp_netif_sntp_deinit();
}