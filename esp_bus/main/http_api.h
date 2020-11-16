/* Declarations for http_api.c */
#include "esp_system.h" //esp_err_t

#define EMT_MAX_LINE_LEN 10
typedef struct {
   int number;     // bus id number
   int time;       // time in seconds to arrive
   int distance;   // distance in meters to stop
   char line[EMT_MAX_LINE_LEN];  // line (might be alphanumeric)
} bus_t;


esp_err_t emt_login(char *buffer, size_t len);
int emt_arrive_times(char *token, bus_t *buses, size_t maxbuses);


#define OK 0
#define FAIL -1
