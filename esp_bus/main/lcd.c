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

    if (distance < 1100) {
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

