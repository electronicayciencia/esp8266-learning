/* Declarations for lcd.c */

#include "esp_system.h" //esp_err_t

#define LCD_LINES 4
#define LCD_LEN 20*LCD_LINES
#define LCD_I2C_ADDR 0x27

void lcd_wait_new_data(void);
void lcd_data_unstable(void);
void lcd_new_data(void);
esp_err_t lcd_initialise(int scl, int sda);
void update_lcd_physical(char *ram); 
void update_lcd_text();

void format_elapsed(int s, char *out);
void format_busnumber(int number, char *out);
void format_time(int time, char *out);
void format_distance(int distance, char *out);
void format_busstop(int number, char *out);
void format_busline(char *line, char *out);

// LCD lines are interleaved
// 20 characters. Line order is 1,3,2,4
#define LCD_LINE_1 0
#define LCD_LINE_3 20*1
#define LCD_LINE_2 20*2
#define LCD_LINE_4 20*3
