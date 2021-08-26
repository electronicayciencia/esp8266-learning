#ifndef ESPNOW_EXAMPLE_H
#define ESPNOW_EXAMPLE_H

typedef struct {
    uint8_t mac_addr[ESP_NOW_ETH_ALEN];
    uint8_t *data;
    int data_len;
} received_data_evt_t;

#endif