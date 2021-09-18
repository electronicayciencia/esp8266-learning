/* Simplified ESP-NOW Tx */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "esp_sleep.h"
#include "esp_system.h"
#include "ds1820.h"
#include "driver/adc.h"

// LED logic is inverted
/*
#define LED_ON  0
#define LED_OFF 1
#define LED_BITMASK GPIO_Pin_2
#define LED_GPIO GPIO_NUM_2
*/

#define PIN_1WIRE  GPIO_NUM_2

static const char *TAG = "espnow-tx";
static uint8_t broadcast_mac[ESP_NOW_ETH_ALEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };


static esp_err_t read_vcc(uint16_t *vcc_value) {
    ESP_LOGI(TAG, "adc read start");
    // init adc and read
    adc_config_t adc_config;

    // Depend on menuconfig->Component config->PHY->vdd33_const value
    // When measuring system voltage(ADC_READ_VDD_MODE), vdd33_const must be set to 255.
    adc_config.mode = ADC_READ_VDD_MODE;
    adc_config.clk_div = 8; // ADC sample collection clock = 80MHz/clk_div = 10MHz

    ESP_ERROR_CHECK(adc_init(&adc_config));

    if (ESP_OK == adc_read(vcc_value)) {
        ESP_LOGI(TAG, "VCC (mV): %d", *vcc_value);
        return ESP_OK;
    }

}


/* WiFi should start before using ESPNOW */
/* Also, read ADC just before strating wifi */
static esp_err_t wifi_init_and_vcc_read(uint16_t *vcc_value)
{
    tcpip_adapter_init();

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );

    ESP_ERROR_CHECK( read_vcc(vcc_value) );

    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_start());
    ESP_ERROR_CHECK( esp_wifi_set_channel(CONFIG_ESPNOW_CHANNEL, 0) );
    return ESP_OK;
}


/* Always ESP_NOW_SEND_SUCCESS on broadcasting mode */
static void my_espnow_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status) {
    ESP_LOGD(TAG, "Data sent.");
}

/* Can receive data from other modules */
static void my_espnow_recv_cb(const uint8_t *mac_addr, const uint8_t *data, int len) {
    ESP_LOGD(TAG, "Data received");
}


static esp_err_t espnow_init(void)
{
    /* Initialize ESPNOW and register sending and receiving callback function. */
    ESP_ERROR_CHECK( esp_now_init() );
    ESP_ERROR_CHECK( esp_now_register_send_cb(my_espnow_send_cb) );
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

static void teardown(void) {
    esp_now_deinit();
    esp_wifi_stop();
    ESP_LOGI(TAG, "Sleep: %ds.\n", CONFIG_MEASUREMENT_INTERVAL);
    esp_deep_sleep(CONFIG_MEASUREMENT_INTERVAL * 1e6);
}




void app_main()
{
    // Prepare GPIO for blinking blue LED in ESP-01S
    /*
    gpio_config_t io_conf = {
        .pin_bit_mask = LED_BITMASK,
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&io_conf);
    gpio_set_level(LED_GPIO, LED_ON);
    */

    ds1820_device_t *dev = ds1820_init(PIN_1WIRE, DS1820_ROM_UNKNOWN);

    if (dev == NULL) {
        ESP_LOGE(TAG, "No DS1820 device found.");
        teardown();
    }

    float temperature;
    ds1820_err_t result = ds1820_read_temp(dev, &temperature);
    
    if (result != DS1820_ERR_OK) {
        ESP_LOGE(TAG, "Error reading DS1820.");
        teardown();
    }


    // Initialize NVS, wifi and esp_now
    uint16_t vcc_value;
    ESP_ERROR_CHECK( nvs_flash_init() );
    ESP_ERROR_CHECK( wifi_init_and_vcc_read(&vcc_value) );
    ESP_ERROR_CHECK( espnow_init() );

    char *data = malloc(sizeof(char) * ESP_NOW_MAX_DATA_LEN);
    //no float format compiled in SDK if 'nano' formatting options enabled
    snprintf(data, ESP_NOW_MAX_DATA_LEN, "t=%.3f,v=%d", temperature, vcc_value);
    
    esp_err_t ret = esp_now_send(broadcast_mac, (uint8_t*) data, strlen(data) + 1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Send error: %d", ret);
    }
    else {
        ESP_LOGI(TAG, "Data sent: %s", data);
    }

    //gpio_set_level(LED_GPIO, LED_OFF);

    teardown();
}
