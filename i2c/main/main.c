/* Read PCF8591 registers using I2C.

*/
#include <stdio.h> // printf
#include "freertos/FreeRTOS.h" // portTICK_PERIOD_MS
#include "freertos/task.h" // vTaskDelay
#include "esp_log.h" // ESP_LOGI

#include "driver/i2c.h"

#define LOG_TAG "i2c"
#define I2C_SCL_IO 0        
#define I2C_SDA_IO 2       
#define I2C_NUM    0

#define I2C_PCF8591_ADDRESS 0x48
#define PCF8591_REGISTER_NTC   0
#define PCF8591_REGISTER_LDR   1
#define PCF8591_REGISTER_INPUT 2
#define PCF8591_REGISTER_POT   3


#define I2C_PCF8591_READ  (I2C_PCF8591_ADDRESS << 1 | I2C_MASTER_READ)
#define I2C_PCF8591_WRITE (I2C_PCF8591_ADDRESS << 1 | I2C_MASTER_WRITE)


static esp_err_t i2c_init() {
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_SDA_IO;
    conf.sda_pullup_en = 1;
    conf.scl_io_num = I2C_SCL_IO;
    conf.scl_pullup_en = 1;
    conf.clk_stretch_tick = 100; // what is this for?
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM, conf.mode));
    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM, &conf));
    return ESP_OK;
}

static esp_err_t read_PCF8591_register(uint8_t reg, uint8_t *outval) {
	esp_err_t ret;
	i2c_cmd_handle_t cmd;

	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, I2C_PCF8591_WRITE, 1);
	i2c_master_write_byte(cmd, reg, 1);
	//i2c_master_stop(cmd);
	ret = i2c_master_cmd_begin(I2C_NUM, cmd, 2 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);

    if (ret != ESP_OK) {
        return ret;
    }

	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, I2C_PCF8591_READ, 1);
	i2c_master_read_byte(cmd, outval, I2C_MASTER_ACK); // ignore first output
	i2c_master_read_byte(cmd, outval, I2C_MASTER_NACK);
	i2c_master_stop(cmd);
	ret = i2c_master_cmd_begin(I2C_NUM, cmd, 2 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);

	return ret;
}


void app_main()
{
	uint8_t val;
	ESP_ERROR_CHECK(i2c_init());

	while(true) {
		if (read_PCF8591_register(PCF8591_REGISTER_LDR, &val) != ESP_OK) {
			ESP_LOGE(LOG_TAG, "Error reading I2C device.");
			vTaskDelay(1000 / portTICK_RATE_MS);
		}
		else {
			printf("LDR: %u\n", val);
			vTaskDelay(100 / portTICK_RATE_MS);
		}
	}
}

