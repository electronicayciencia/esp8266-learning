/* 

EMT bus monitor.

Electronicayciencia 2020/11/13.

Based on https_request example.

*/
#define TAG "main"

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "esp_http_client.h"
#include "cJSON.h"


#include "esp_bus.h"
#include "config.h"
#include "wifi.h"

#define MAX_HTTP_RESPONSE_SIZE 2048


char http_response[MAX_HTTP_RESPONSE_SIZE];

/* Root certificate */
extern const char httpbin_pem_start[] asm("_binary_httpbin_ca_pem_start");
extern const char emtmadrid_pem_start[] asm("_binary_emtmadrid_ca_pem_start");


esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    static int readed = 0; // index inside http_response buffer

    switch(evt->event_id) {
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            readed = 0;
            break;

        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);

            int remaining = MAX_HTTP_RESPONSE_SIZE - readed - 1;
            int tocopy = 
                remaining > evt->data_len ? evt->data_len : remaining;

            memcpy(http_response + readed, evt->data, tocopy);
            readed += tocopy;
            http_response[readed] = 0; // zero terminated for now
            ESP_LOGD(TAG, "Copied %d bytes.", tocopy);
            break;

        default:
            break;
    }

    return ESP_OK;
}



/* Try to get a token from EMT API.
   Parameters:
      buffer: buffer to put the token into
      len: length of the buffer
   Return:
      ESP_OK if successful
   Globals:
      emtmadrid_pem_start
      http_response
 */
static esp_err_t emt_login(char *buffer, size_t len) {
    esp_http_client_config_t config = {
        .url = EMT_LOGIN_URL,
        .event_handler = _http_event_handler,
        .cert_pem = emtmadrid_pem_start,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_http_client_set_header(client, EMT_LOGIN_CLIENT_HEADER, CONFIG_EMT_OPENAPI_CLIENTID);
    esp_http_client_set_header(client, EMT_LOGIN_PASSWORD_HEADER, CONFIG_EMT_OPENAPI_PASSKEY);

    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        size_t r_size = esp_http_client_get_content_length(client);
        int r_status  = esp_http_client_get_status_code(client);

        esp_http_client_cleanup(client);

        ESP_LOGI(TAG, "HTTPS Status = %d, content_length = %d",
                r_status, r_size);

        if (r_size >= MAX_HTTP_RESPONSE_SIZE) {
            ESP_LOGE(TAG, 
                "HTTP response was too big :( (size: %d, max: %d)",
                r_size, MAX_HTTP_RESPONSE_SIZE);
            return ESP_FAIL;
        }

        if (r_status != 200) {
            ESP_LOGE(TAG, "HTTP status error: %d", r_status);
            return ESP_FAIL;
        }

    } else {
        ESP_LOGE(TAG, "Error perform http request %d", err);
        return ESP_FAIL;
    }

    cJSON *api_response = cJSON_Parse(http_response);
    if (api_response == NULL) {
        ESP_LOGE(TAG, "Cannot parse JSON response:\n%s\n", http_response);
        cJSON_Delete(api_response);
        return ESP_FAIL;
    }

    cJSON *code = cJSON_GetObjectItemCaseSensitive(api_response, "code");
    if (code == NULL || 
          (code->valueint != EMT_CODE_OK && 
           code->valueint != EMT_CODE_LOGIN_OK)) {
        ESP_LOGE(TAG, "EMT API code error:\n%s\n", http_response);

        cJSON_Delete(api_response);
        return ESP_FAIL;
    }

    cJSON *data = cJSON_GetObjectItemCaseSensitive(api_response, "data");
    if (data == NULL || 
          !cJSON_IsArray(data) || 
          !data->child || 
          !cJSON_IsObject(data->child)) {
        ESP_LOGE(TAG, "EMT API response error:\n%s\n", http_response);
        cJSON_Delete(api_response);
        return ESP_FAIL;
    }
    
    cJSON *token = cJSON_GetObjectItemCaseSensitive(data->child, "accessToken");
    if (token == NULL || !cJSON_IsString(token)) {
        ESP_LOGE(TAG, "EMT API no token error:\n%s\n", http_response);
        cJSON_Delete(api_response);
        return ESP_FAIL;
    }

    strncpy(buffer, cJSON_GetStringValue(token), len);
    cJSON_Delete(api_response);

    return ESP_OK;
}


static void https_get_task(void *pvParameters) {
    char token[EMT_TOKEN_LEN];

    while(1) {
        ESP_LOGI(TAG, "Waiting for WiFi...");
        wifi_wait_connected();        
        ESP_LOGI(TAG, "Connected to AP!");

        
        if (emt_login(token, EMT_TOKEN_LEN) == ESP_OK) {
            ESP_LOGI(TAG, "Got access token: %s", token);
        }
        else {
            ESP_LOGE(TAG, "Login call failed.");
        }


        for(int countdown = 10; countdown >= 0; countdown--) {
            ESP_LOGI(TAG, "%d...", countdown);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        ESP_LOGI(TAG, "Starting again!");
    }
}

void app_main() {
    ESP_ERROR_CHECK( nvs_flash_init() );
    wifi_initialise();
    xTaskCreate(&https_get_task, "https_get_task", 8192, NULL, 5, NULL);
}

