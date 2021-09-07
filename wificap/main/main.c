/* Packet capturer

pcap header
- X times
  frame header
  radiotap header
  frame bytes
-

*/

#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_event.h"
#include "nvs_flash.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_libc.h"
#include "packetheaders.h"

#define TAG "sniffer"

#define MAC_HEADER_LEN 24
#define SNIFFER_DATA_LEN 112
#define MAC_HDR_LEN_MAX 40

static EventGroupHandle_t wifi_event_group;

static const int START_BIT = BIT0;


static void hexaprint(void *structure, size_t size)
{
    for (int i=0; i < size; i++) {
        printf("%02X ", *((uint8_t*)structure + i));
    }
}

static void print_pcap_header()
{
    pcap_header_t pcaphead = {
        .magic = PCAP_MAGIC_NUMBER_US,
        .mayor = PCAP_VERSION_MAYOR,
        .minor = PCAP_VERSION_MINOR,
        .snaplen = 0x40000,
        .linktype = LINKTYPE_IEEE802_11_RADIOTAP,
    };

    hexaprint(&pcaphead, sizeof(pcaphead));
    puts("");
}


static void sniffer_cb(void* buf, wifi_promiscuous_pkt_type_t type)
{
    wifi_pkt_rx_ctrl_t* rx_ctrl = (wifi_pkt_rx_ctrl_t*)buf;
    uint8_t* frame = (uint8_t*)(rx_ctrl + 1);
    uint32_t len = rx_ctrl->sig_mode ? rx_ctrl->HT_length : rx_ctrl->legacy_length;
    uint32_t captured_len;

    uint8_t total_num = 1, count = 0;
    uint16_t seq_buf = 0;

    if ((rx_ctrl->aggregation) && (type != WIFI_PKT_MISC)) {
        total_num = rx_ctrl->ampdu_cnt;
    }

    for (count = 0; count < total_num; count++) {
        if (total_num > 1) {
            len = *((uint16_t*)(frame + MAC_HDR_LEN_MAX + 2 * count));

            if (seq_buf == 0) {
                seq_buf = *((uint16_t*)(frame + 22)) >> 4;
            }

            //ESP_LOGI(TAG, "seq_num:%d, total_num:%d\r\n", seq_buf, total_num);
        }

        switch (type) {
            case WIFI_PKT_MGMT:
            case WIFI_PKT_CTRL:
            case WIFI_PKT_DATA:
                break;

            case WIFI_PKT_MISC:
                len = len > MAC_HEADER_LEN ? MAC_HEADER_LEN : len;
                break;

            default :
                return;
        }

        ++seq_buf;

        if (total_num > 1) {
            *(uint16_t*)(frame + 22) = (seq_buf << 4) | (*(uint16_t*)(frame + 22) & 0xf);
        }
    }

    captured_len = len > SNIFFER_DATA_LEN ? SNIFFER_DATA_LEN : len;

    // TODO: Get fields from rx control header
    radiotap_header_flags_t radiohead_flags = {
        .flags = RADIOTAP_FLAGS_INCLUDE_FCS, // TODO: check that with control header
    };
    radiotap_header_t radiohead = {
        .version = RADIOTAP_HEADER_VERSION,
        .header_size = sizeof(radiohead) + sizeof(radiohead_flags),
        .present_fields = RADIOTAP_PRESENT_FIELDS_FLAGS,
    };


    int64_t timestamp_us = esp_timer_get_time();

    frame_header_t framehead = {
        .timestamp = timestamp_us / 1e6,
        .microseconds = timestamp_us % (int64_t)1e6,
        .size_file = captured_len + sizeof(radiohead) + sizeof(radiohead_flags),
        .size_wire = len + sizeof(radiohead) + sizeof(radiohead_flags),
    };

    hexaprint(&framehead, sizeof(framehead));
    puts("");

    hexaprint(&radiohead, sizeof(radiohead));
    hexaprint(&radiohead_flags, sizeof(radiohead_flags));
    puts("");

    hexaprint(frame, captured_len);
    puts("");
    puts("");
}

static void sniffer_task(void* pvParameters)
{
    wifi_promiscuous_filter_t sniffer_filter = {0};

#if CONFIG_FILTER_MASK_MGMT
    sniffer_filter.filter_mask |= WIFI_PROMIS_FILTER_MASK_MGMT;
#endif

#if CONFIG_FILTER_MASK_CTRL
    sniffer_filter.filter_mask |= WIFI_PROMIS_FILTER_MASK_CTRL;
#endif

#if CONFIG_FILTER_MASK_DATA
    sniffer_filter.filter_mask |= WIFI_PROMIS_FILTER_MASK_DATA;
#endif

    /*Enable to receive the correct data frame payload*/
    extern esp_err_t esp_wifi_set_recv_data_frame_payload(bool enable_recv);
    ESP_ERROR_CHECK(esp_wifi_set_recv_data_frame_payload(true));

#if CONFIG_FILTER_MASK_MISC
    sniffer_filter.filter_mask |= WIFI_PROMIS_FILTER_MASK_MISC;
#endif

    if (sniffer_filter.filter_mask == 0) {
        ESP_LOGI(TAG, "Please add one filter at least!");
        vTaskDelete(NULL);
    }

    print_pcap_header();

    xEventGroupWaitBits(wifi_event_group, START_BIT,
                        false, true, portMAX_DELAY);
    ESP_ERROR_CHECK(esp_wifi_set_channel(CONFIG_CHANNEL, 0));
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous_rx_cb(sniffer_cb));
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous_filter(&sniffer_filter));
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));
    vTaskDelete(NULL);
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_STA_START) {
        xEventGroupSetBits(wifi_event_group, START_BIT);
    }
}

static void initialise_wifi(void)
{
    tcpip_adapter_init();

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_event_group = xEventGroupCreate();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void app_main()
{
    ESP_ERROR_CHECK(nvs_flash_init());
    initialise_wifi();
    xTaskCreate(&sniffer_task, "sniffer_task", 2048, NULL, 10, NULL);
}