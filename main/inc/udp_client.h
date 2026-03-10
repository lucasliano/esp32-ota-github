
/**
 * @file upd_client.h
 * @brief Definition of wifi related functions.
 */

#ifndef UDP_CLIENT_H
#define UDP_CLIENT_H

// --- Generic includes ---
#include "common.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/ringbuf.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "esp_log.h"

// --- Project related includes ---

// --- Generic defines ---
#define UDP_SERVER_IP "172.16.0.171"
#define UDP_SERVER_PORT 5005
#define CIRCULAR_BUFFER_SIZE (4 * 1024)
#define RETRY_TIMEOUT 1 


// --- Function definitions ---
void syslog_enqueue(const char *msg, int len);
void syslog_udp_start(void);

#endif // UDP_CLIENT_H
