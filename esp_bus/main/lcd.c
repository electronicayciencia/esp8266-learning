#define TAG "lcd"

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "esp_event_loop.h"
#include "freertos/event_groups.h"
#include "esp_log.h"

#include "soft_lcd.h"
#include "lcd.h"

#define LCD_DATA_READY  BIT0


static EventGroupHandle_t lcd_event_group;
static lcd_t *lcd;


void lcd_data_unstable() {
    xEventGroupClearBits(lcd_event_group, LCD_DATA_READY);
}

void lcd_data_stable() {
    xEventGroupSetBits(lcd_event_group, LCD_DATA_READY);
}

void lcd_wait_data_stable() {
    xEventGroupWaitBits(lcd_event_group, LCD_DATA_READY, true, true, portMAX_DELAY);
}

esp_err_t lcd_initialise(int scl, int sda) {
    lcd = lcd_create(scl, sda, LCD_I2C_ADDR, LCD_LINES);
    if (lcd == NULL) {
        return ESP_FAIL;
    }

    lcd_event_group = xEventGroupCreate();
    ESP_LOGI(TAG, "LCD set-up OK.");

    return ESP_OK;
}


/* Copy LCD RAM to physical LCD 
   Globals:
    lcd_t *lcd: initialized LCD structure
*/
void update_lcd_physical(char *ram) {
    int i;

    lcd_pos(lcd,0,0);

    for (i = 0; i < LCD_LEN; i++) {
        if (ram[i] == 0) {
            lcd_putc(lcd, '\\'); // null => blank (space)
        }
        else {
            lcd_putc(lcd, ram[i]);
        }
    }
}


/* Format the bus number and put it in a memory address.
   Input: 
     busnumber
     buffer (5 pos) non zero terminated
   Output format examples:
   ··123
*/
void format_busnumber(int number, char *out) {
    char buff[6]; // this is zero terminated
    snprintf(buff, 6, "%5d", number);
    memcpy(out, buff, 5);
}



/* Format the distance and put it in a memory address.
   Input: 
     meters
     buffer (5 pos) non zero terminated
   Output format examples:
   ·500m   (1099 or below)
   2.5km   (between 1100 and 9999)
   ·10km   (between 10000 and 19999)
   >20km   (20000 or above)
*/
void format_distance(int distance, char *out) {
    char buff[6]; // this is zero terminated
    int km = distance/1000;
    int hm = (distance - 1000*km) / 100;

    if (distance < 0) {
        snprintf(buff, 6, "  ??m");
    }
    else if (distance >= 0 && distance < 1100) {
        snprintf(buff, 6, "%4dm", distance);
    }
    else if (distance >= 1100 && distance < 10000) {
        snprintf(buff, 6, "%d.%01dkm", km, hm);
    }
    else if (distance >= 10000 && distance < 20000) {
        snprintf(buff, 6, "%3dkm", km);
    }
    else {
        snprintf(buff, 6, " >20km");
    }

    memcpy(out, buff, 5);
}


/* Format the arrival time and put it in a memory address.
   Input: 
     seconds
     buffer (6 pos) non zero terminated
   Output format examples:
   ····9s  (59s or below)
   9m 01s  (between 60 and 10*60s - 1)
   ···12m  (between 600 and 15*60s - 1)
   ··>15m  (15*60s or above)
*/
void format_time(int time, char *out) {
    char buff[7]; // this is zero terminated
    int m = time/60;
    int s = time - 60*m;

    if (time < 60) {
        snprintf(buff, 7, "%5ds", s);
    }
    else if (time >= 60 && time < 10*60) {
        snprintf(buff, 7, "%dm %02ds", m, s);
    }
    else if (time >= 10*60 && time < 15*60) {
        snprintf(buff, 7, "%5dm", m);
    }
    else {
        snprintf(buff, 7, "  >15m");
    }

    memcpy(out, buff, 6);
}

/* Format the elapsed number of seconds.
   Input: 
     seconds
     buffer (3 pos) non zero terminated
   Output format examples:
   1s· (59s or below)
   1m· (between 60 and 9*60 - 1 )
   >9m (9*60s or above)

*/
void format_elapsed(int s, char *out) {
    char buff[4]; // this is zero terminated
    int m = s/60;

    if (s < 60) {
        snprintf(buff, 4, "%ds", s);
    }
    else if (s >= 60 && s < 9*60) {
        snprintf(buff, 4, "%dm", m);
    }
    else {
        snprintf(buff, 4, ">9m");
    }

    // fill final space if needed
    if (buff[2] == 0)
        buff[2] = ' ';

    memcpy(out, buff, 3);
}

/* Format the bus stop and put it in a memory address.
   Input: 
     busstop
     buffer (4 pos) non zero terminated
   Output format examples:
   ··123
*/
void format_busstop(int number, char *out) {
    char buff[5]; // this is zero terminated
    snprintf(buff, 5, "%4d", number);
    memcpy(out, buff, 4);
}

/* Format the bus line and put it in a memory address.
   Input: 
     line
     buffer (3 pos) non zero terminated
   Output format examples:
   23·
*/
void format_busline(char *line, char *out) {
    char buff[4]; // this is zero terminated
    snprintf(buff, 4, "%3s", line);
    memcpy(out, buff, 3);
}
