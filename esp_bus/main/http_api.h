/* Declarations for http_api.c */
#include "esp_system.h" //esp_err_t
 
esp_err_t emt_login(char *buffer, size_t len);
int emt_arrive_times(char *token, Bus *buses, size_t maxbuses);