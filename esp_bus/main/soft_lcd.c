/*
 * soft_lcd
 *      Library to use PCF8574 based LCD via I2C bus.
 *
 *      Electronica y ciencia.
 *      https://electronicayciencia.blogspot.com/
 *
 *      Reinoso G.
 */

#define TAG "soft_lcd"
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>

#include "freertos/FreeRTOS.h" // portTICK_PERIOD_MS
#include "freertos/task.h" // vTaskDelay
#include "driver/i2c.h"
#include "esp_log.h"

#include "soft_lcd.h"

static int i2c_init(int scl, int sda) {
	int i2c_num = 0; // only one port for now

    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = sda;
    conf.sda_pullup_en = 1;
    conf.scl_io_num = scl;
    conf.scl_pullup_en = 1;
    conf.clk_stretch_tick = 100; // what is this for?
    ESP_ERROR_CHECK(i2c_driver_install(i2c_num, conf.mode));
    ESP_ERROR_CHECK(i2c_param_config(i2c_num, &conf));
    return i2c_num;
}

lcd_t *lcd_create(int scl, int sda, int addr, int lines) {

	int i2c = i2c_init(scl, sda);
	if ( !_pcf8574_check(i2c, addr) ) return NULL;

	lcd_t *lcd = (lcd_t*) malloc(sizeof(lcd_t));

	lcd->_addr         = addr;
	lcd->_i2c          = i2c;
	lcd->_lines        = lines;
	lcd->_dimming      = -1; // dimmer no used yet

	lcd->err           = 0;

	lcd->fcn_set       = LCD_FCN_4BIT | LCD_FCN_5x8;

	if (lines > 1)
		 lcd->fcn_set |= LCD_FCN_2LINES;

	lcd->cursor_set    = LCD_CURSOR_MOVE_CUR | LCD_CURSOR_LEFT;
	lcd->display_set   = LCD_DISPLAY_ON | LCD_DISPLAY_CURSOR_OFF | LCD_DISPLAY_BLINK_OFF;
	lcd->entrymode_set = LCD_ENTRYMODE_CURSOR_DECR | LCD_ENTRYMODE_SCROLL_OFF;

	lcd->backlight     = LCD_BKLIGHT;
	lcd->replace_UTF8_chars = 1;
	
	lcd_reset(lcd);
	lcd_init(lcd);

	return lcd;
}

void lcd_destroy(lcd_t *lcd) {
	lcd_off(lcd);
	lcd_backlight_off(lcd);
	lcd_reset(lcd);
	i2c_driver_delete(lcd->_i2c);
	free(lcd);
}


void lcd_init(lcd_t *lcd) {
	lcd_reconfig(lcd);
	lcd_clear(lcd);
	lcd_home(lcd);
}


/* send configuration parameters to LCD */
void lcd_reconfig(lcd_t *lcd) {
	lcd_reconfig_fcn(lcd);
	lcd_reconfig_cursor(lcd);
	lcd_reconfig_display(lcd);
	lcd_reconfig_entrymode(lcd);
}

void lcd_reconfig_fcn(lcd_t *lcd) { 
	lcd_raw(lcd, LCD_WRITE, LCD_CMD_FCN_SET | lcd->fcn_set);
}
void lcd_reconfig_cursor(lcd_t *lcd) {
	lcd_raw(lcd, LCD_WRITE, LCD_CMD_CURSOR_SET | lcd->cursor_set);
}
void lcd_reconfig_display(lcd_t *lcd) {
	lcd_raw(lcd, LCD_WRITE, LCD_CMD_DISPLAY_SET | lcd->display_set);
}
void lcd_reconfig_entrymode(lcd_t *lcd) {
	lcd_raw(lcd, LCD_WRITE, LCD_CMD_ENTRYMODE_SET | lcd->entrymode_set);
}

void lcd_home (lcd_t *lcd) {
	lcd_raw(lcd, LCD_WRITE, LCD_CMD_HOME);
}

void lcd_clear (lcd_t *lcd) {
	lcd_raw(lcd, LCD_WRITE, LCD_CMD_CLEAR);
	vTaskDelay(10 / portTICK_PERIOD_MS);
	//ESP_LOGD("i2cli","t2");
}

/* Convenient shortcuts */
void lcd_on(lcd_t *lcd) {
	lcd->display_set |= LCD_DISPLAY_ON;
	lcd_reconfig_display(lcd);
}

void lcd_off(lcd_t *lcd) {
	lcd->display_set &= ~LCD_DISPLAY_ON;
	lcd_reconfig_display(lcd);
}

void lcd_backlight_on(lcd_t *lcd) {
	lcd->backlight = LCD_BKLIGHT;
	lcd_reconfig_display(lcd);
}

void lcd_backlight_off(lcd_t *lcd) {
	lcd->backlight = 0;
	lcd_reconfig_display(lcd);
}

void lcd_cursor_on(lcd_t *lcd) {
	lcd->display_set |= LCD_DISPLAY_CURSOR_ON;
	lcd_reconfig_display(lcd);
}

void lcd_cursor_off(lcd_t *lcd) {
	lcd->display_set &= ~LCD_DISPLAY_CURSOR_ON;
	lcd_reconfig_display(lcd);
}

void lcd_blink_on(lcd_t *lcd) {
	lcd->display_set |= LCD_DISPLAY_BLINK_ON;
	lcd_reconfig_display(lcd);
}

void lcd_blink_off(lcd_t *lcd) {
	lcd->display_set &= ~LCD_DISPLAY_BLINK_ON;
	lcd_reconfig_display(lcd);
}

void lcd_pos(lcd_t *lcd, int row, int col) {
	int rows_value[] = {0x00, 0x40, 0x14, 0x54};
	lcd_pos_raw(lcd, rows_value[row] + col);
}

void lcd_pos_raw(lcd_t *lcd, int pos) {
	lcd_raw(lcd, LCD_WRITE, LCD_CMD_DDRAM_SET | pos);
}

/* Set LCD controller into a known state and set 4 bit mode */
void lcd_reset (lcd_t *lcd) {
	//ESP_LOGD("i2cli","t45");
	_pcf8574_put(lcd, LCD_D5 | LCD_D4);
	vTaskDelay(45 / portTICK_PERIOD_MS);

	//ESP_LOGD("i2cli","t5");
	_pcf8574_put(lcd, LCD_D5 | LCD_D4);
	vTaskDelay(20 / portTICK_PERIOD_MS);

	//ESP_LOGD("i2cli","t2");
	_pcf8574_put(lcd, LCD_D5 | LCD_D4);
	vTaskDelay(20 / portTICK_PERIOD_MS);


	/* we assume pcf8574 and 4bit mode for now */
	if (lcd->fcn_set & LCD_FCN_8BIT) return;

	_pcf8574_put(lcd, LCD_CMD_FCN_SET | LCD_FCN_4BIT);
	vTaskDelay(20 / portTICK_PERIOD_MS);

}

/* Printf to LCD screen  */
void lcd_printf(lcd_t *lcd, const char* format, ... ) {
	int linesize = 0x40; // max size in a 16x2 display

	char *buff = (char *) malloc(sizeof(char) * (linesize+1));

	va_list args;
	va_start(args, format);
	vsnprintf(buff, linesize, format, args);
	va_end(args);

	lcd_print(lcd, buff);

	free(buff);
}

/* Prints string in actual cursor position */
void lcd_print(lcd_t *lcd, char *instr) {
	unsigned int i;
	char *s = instr;

	if (lcd->replace_UTF8_chars) s = _replace_UTF8_chars(instr);

	for (i = 0; i < strlen(s); i++) {
		if (s[i] == '\n') {
			_lcd_nextline(lcd);
		}
		else {
			//ESP_LOGD(TAG, "Char: %02x\n", s[i]);
			lcd_raw(lcd, LCD_WRITE | LCD_RS, s[i]);
		}
	}

	if (lcd->replace_UTF8_chars) free(s);
}

/* Create characters in the CGRAM table
 * Note that character 0 may be defined, but cannot be used because \x00 is
 * not valid inside a string */
void lcd_create_char(lcd_t *lcd, int n, char *data) {
	if (n < 0 || n > 8) return;
	
	int cursor_pos = lcd_read_pos_raw(lcd);

	lcd_raw(lcd, LCD_WRITE, LCD_CMD_CGRAM_SET + 8 * n);

	int i;
	for (i = 0; i < 8; i++) {
		lcd_raw(lcd, LCD_WRITE | LCD_RS, data[i]);
	}

	lcd_pos_raw(lcd, cursor_pos);
}

/* Read cursor pos and busy flag */
int lcd_read_pos_raw (lcd_t *lcd) {
	return lcd_read_raw(lcd, 0);
}

/* Read data at cursor and shift */
int lcd_read_data (lcd_t *lcd) {
	return lcd_read_raw(lcd, LCD_RS);
}

/* Not supported yet in ESP version*/
///* Dim the backlight using PWM on pin BCM 18 */
//void lcd_backlight_dim (lcd_t *lcd, float intensity) {
//	if (lcd->_dimming < 0) {
//		pwmSetClock(LCD_PWM_CLOCK);
//		pwmSetRange(LCD_PWM_RANGE);
//		pinMode(LCD_PWM_PIN, PWM_OUTPUT);
//	}
//
//	if (intensity > 1) intensity = 1;
//	if (intensity < 0) intensity = 0;
//
//	pwmWrite(LCD_PWM_PIN, (int) (intensity * (float)LCD_PWM_RANGE));
//	lcd->_dimming = intensity;
//}

/* Replace non-ascii characters in the string.
 * String must be UTF8.
 * Don't use this if you want actual katakana or
 *  if you have got the european version of HD44780. */
char *_replace_UTF8_chars(char *s) {
	int i = 0; // input counter
	int o = 0; // output counter

	char *t = (char*) malloc(sizeof(char) * (strlen(s) + 1));

	while (s[i]) {

		t[o] = s[i];

		if (s[i] == 0b11000011) {
			switch (s[i+1]) {
				case 0x81: t[o] = 'A'; break; // Á
				case 0x89: t[o] = 'E'; break; // É
				case 0x8d: t[o] = 'I'; break; // Í
				case 0x93: t[o] = 'O'; break; // Ó
				case 0x9a: t[o] = 'U'; break; // Ú
				case 0x91: t[o] = 0xee; break; // ñ
				case 0xa1: t[o] = 'a'; break; // á
				case 0xa9: t[o] = 'e'; break; // é
				case 0xad: t[o] = 'i'; break; // í
				case 0xb3: t[o] = 'o'; break; // ó
				case 0xba: t[o] = 'u'; break; // ú
				case 0xb1: t[o] = 0xee; break; // ñ
				default:   i--; break; // not interesting
			}
			i++;
		} else if (s[i] == 0b11000010) {
			switch (s[i+1]) {
				case 0xBA: t[o] = 0xdf; break; // º
				default:   i--; break; // not interesting
			}
			i++;
		}
		o++;
		i++;
	}
	t[o] = 0;

	return t;
}

/* Reads the cursor position and sits it at
 * the beginning of the next line */
void _lcd_nextline(lcd_t *lcd) {
	int pos = lcd_read_pos_raw(lcd);
	//printf("Cursor was at %d.\n", lcd_read_pos_raw(lcd));

	/* LCD should not be busy now */
	if (pos & LCD_BUSY_FLAG) {
		lcd->err = 1;
		return;
	}

	/* Different LCD lines have different ranges */
	switch (lcd->_lines) {
		case 1:
			lcd_pos_raw(lcd, 0x00);
			break;
		case 2:
			if (pos < 0x40)
				lcd_pos_raw(lcd, 0x40);
			break;
		case 4:
			if (pos < 0x14)                     // first line
				lcd_pos_raw(lcd, 0x40);
			else if (pos >= 0x40 && pos < 0x54) // second line
				lcd_pos_raw(lcd, 0x14);
			else if (pos >= 0x14 && pos < 0x40) // third line
				lcd_pos_raw(lcd, 0x54);
			break;
	}
}

/* Writting a raw command */
void lcd_raw (lcd_t *lcd, int lcd_opts, int data) {
	int upper = (data >> 4) & 0xF;
	int lower = data & 0xF;

	lcd_opts |= lcd->backlight;

	//ESP_LOGD(TAG, "Data: %02x\n", data);

	_pcf8574_put(lcd, (upper << 4) | lcd_opts);
	_pcf8574_put(lcd, (lower << 4) | lcd_opts);
}

/* Reading a raw byte */
int lcd_read_raw (lcd_t *lcd, int rs) {
	int u = _pcf8574_get(lcd, rs);
	int l = _pcf8574_get(lcd, rs);

	return  (u<<4) + l;
}

/* Send a nibble and status lines to PCF8574
 * Sets error condition in lcd if any command is not ack'ed
 * For write operations, the command is executed after enabled falling edge */
void _pcf8574_put (lcd_t *lcd, int lines) {
	esp_err_t ret;
	i2c_cmd_handle_t cmd;

	//ESP_LOGD("i2cli", "s");
	//ESP_LOGD("i2cli","w%02x", lcd->_addr << 1 | I2C_WRITE);
	//ESP_LOGD("i2cli","w%02x", lines | LCD_ENABLED);
	//ESP_LOGD("i2cli","w%02x", lines);
	//ESP_LOGD("i2cli","p");


	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, lcd->_addr << 1 | I2C_WRITE, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, lines | LCD_ENABLED, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, lines, ACK_CHECK_EN);
	i2c_master_stop(cmd);
	ret = i2c_master_cmd_begin(lcd->_i2c, cmd, 2 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);

	if (ret != ESP_OK) {
		lcd->err = LCD_ERR_I2C;
		ESP_LOGD(TAG, "LCD error _pcf8574_put");

		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_stop(cmd);
		i2c_master_cmd_begin(lcd->_i2c, cmd, 2 / portTICK_RATE_MS);
		i2c_cmd_link_delete(cmd);
	}

	return;
}

/* Reads a nibble from data lines of PCF8574
 * For read operations:
 *   - set data lines as input writing them 1
 *   - raise R/W while Enabled is still down
 *   - raise Enabled
 *   - read value
 *   - low enable
 * To read data under cursor, set RS
 * TODO: Set error condition in lcd if any command is not ack'ed */
int _pcf8574_get (lcd_t *lcd, int rs) {
	esp_err_t ret;
	i2c_cmd_handle_t cmd;

	uint8_t byte;
	int lines = LCD_D4|LCD_D5|LCD_D6|LCD_D7
		| LCD_READ
		| rs
		| lcd->backlight;

	cmd = i2c_cmd_link_create();

	/* Set reading lines and reading mode */
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, lcd->_addr << 1 | I2C_WRITE, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, lines, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, lines | LCD_ENABLED, ACK_CHECK_EN);
	i2c_master_stop(cmd);

	/* Actually read lines */
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, lcd->_addr << 1 | I2C_READ, ACK_CHECK_EN);
	i2c_master_read_byte(cmd, &byte, I2C_MASTER_NACK);
	i2c_master_stop(cmd);

	/* Unset read mode */
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, lcd->_addr << 1 | I2C_WRITE, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, lines & ~LCD_ENABLED, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, lines & ~LCD_READ, ACK_CHECK_EN);
	i2c_master_stop(cmd);

	ret = i2c_master_cmd_begin(lcd->_i2c, cmd, 2 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);

    if (ret != ESP_OK) {
        lcd->err = LCD_ERR_I2C;
		ESP_LOGD(TAG, "LCD error _pcf8574_get");

		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_stop(cmd);
		i2c_master_cmd_begin(lcd->_i2c, cmd, 2 / portTICK_RATE_MS);
		i2c_cmd_link_delete(cmd);
    }

	//printf("Readed byte: %02x\n", byte);
	return byte >> 4;
}


/* check if PCF8574 driver is ready */
int _pcf8574_check (int i2c, int addr) {
	
	i2c_cmd_handle_t cmd;
	esp_err_t ret;

	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, addr << 1 | I2C_WRITE, ACK_CHECK_EN);
	i2c_master_stop(cmd);
	ret = i2c_master_cmd_begin(i2c, cmd, 2 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	
	/* Force bus stop */
	if (ret != ESP_OK) {
		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_stop(cmd);
		i2c_master_cmd_begin(i2c, cmd, 2 / portTICK_RATE_MS);
		i2c_cmd_link_delete(cmd);
		return 0;
	}
	
	return 1;
}

