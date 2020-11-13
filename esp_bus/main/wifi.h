/* Declarations for wifi.c */
#include "esp_wifi.h"

esp_err_t wifi_event_handler(void *ctx, system_event_t *event);
void initialise_wifi(void);