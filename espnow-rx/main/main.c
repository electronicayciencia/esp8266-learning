/* Simplified ESP-NOW Rx */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_now.h"

#include "main.h"

#define ESPNOW_QUEUE_SIZE 10

static const char *TAG = "espnow-rx";
static uint8_t broadcast_mac[ESP_NOW_ETH_ALEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
static xQueueHandle received_data_queue;

/* WiFi should start before using ESPNOW */
static esp_err_t wifi_init(void)
{
    tcpip_adapter_init();

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_start());

    ESP_ERROR_CHECK( esp_wifi_set_channel(CONFIG_ESPNOW_CHANNEL, 0) );
    return ESP_OK;
}


/* Not used */
/*
static void my_espnow_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status) {
    ESP_LOGD(TAG, "Data sent.");
}
*/

/* Receive data and put it into a queue */
static void my_espnow_recv_cb(const uint8_t *mac_addr, const uint8_t *data, int len) {
    received_data_evt_t evt;

    if (mac_addr == NULL || data == NULL || len <= 0) {
        ESP_LOGE(TAG, "Receive cb arg error");
        return;
    }

    memcpy(evt.mac_addr, mac_addr, ESP_NOW_ETH_ALEN);
    evt.data = malloc(len); // freed on wait_for_data
    if (evt.data == NULL) {
        ESP_LOGE(TAG, "Malloc receive data fail");
        return;
    }
    memcpy(evt.data, data, len);
    evt.data_len = len;
    if (xQueueSend(received_data_queue, &evt, portMAX_DELAY) != pdTRUE) {
        ESP_LOGW(TAG, "Send receive queue fail");
        free(evt.data);
    }
}


static esp_err_t espnow_init(void)
{
    /* Initialize received data queue */
    received_data_queue = xQueueCreate(ESPNOW_QUEUE_SIZE, sizeof(received_data_evt_t));
    if (received_data_queue == NULL) {
        ESP_LOGE(TAG, "Create queue fail");
        return ESP_FAIL;
    }

    /* Initialize ESPNOW and register sending and receiving callback function. */
    ESP_ERROR_CHECK( esp_now_init() );
    //ESP_ERROR_CHECK( esp_now_register_send_cb(my_espnow_send_cb) );
    ESP_ERROR_CHECK( esp_now_register_recv_cb(my_espnow_recv_cb) );

    /* Add broadcast peer information to peer list. */
    esp_now_peer_info_t *peer = malloc(sizeof(esp_now_peer_info_t));

    memset(peer, 0, sizeof(esp_now_peer_info_t));
    peer->channel = CONFIG_ESPNOW_CHANNEL;
    peer->ifidx = ESP_IF_WIFI_STA;
    peer->encrypt = false;
    memcpy(peer->peer_addr, broadcast_mac, ESP_NOW_ETH_ALEN);
    ESP_ERROR_CHECK( esp_now_add_peer(peer) );
    free(peer);

    return ESP_OK;
}

/* Read received data queue */
static void wait_for_data(void *pvParameter) {
    received_data_evt_t evt;

    ESP_LOGI(TAG, "Waiting for data");

    /* Text Only */
    while (xQueueReceive(received_data_queue, &evt, portMAX_DELAY) == pdTRUE) {
        printf("Received %d bytes from "MACSTR"\n>|%s|<\n", evt.data_len, MAC2STR(evt.mac_addr), evt.data);
        free(evt.data);
    }
}



void app_main() {
    // Initialize NVS, wifi and esp_now
    ESP_ERROR_CHECK( nvs_flash_init() );
    ESP_ERROR_CHECK( wifi_init() );
    ESP_ERROR_CHECK( espnow_init() );

    xTaskCreate(wait_for_data, "wait_for_data", 2048, NULL, 4, NULL);
}
