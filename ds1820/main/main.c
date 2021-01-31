/* Multiple DS1820
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"

#include "esp_log.h"
#include "esp_system.h"

#include "ds1820.h"

#define TAG  "main"

#define NUMDEVICES 3

// GPIO_NUM_2 do not seem to work 
#define PIN_1WIRE  GPIO_NUM_0

#define ROM_0 "\x10\x8C\x67\xE3\x01\x08\x00\x30"
#define ROM_1 "\x10\xC4\x54\xE3\x01\x08\x00\x2E"
#define ROM_2 "\x28\xFF\x8C\x7C\x81\x16\x03\x75"

//void not_app_main(void) {
void app_main(void) {

    ds1820_device_t *sensors[NUMDEVICES];

    puts("Multiple DS1820 Test Program for ESP-IDF");

    sensors[0] = ds1820_init(PIN_1WIRE, ROM_0);
    sensors[1] = ds1820_init(PIN_1WIRE, ROM_1);
    sensors[2] = ds1820_init(PIN_1WIRE, ROM_2);

    while (1) {
        float temperature;

        for (int i = 0; i < NUMDEVICES; i++) {

            printf("Device #%d: ", i);

            ds1820_err_t result = ds1820_read_temp(sensors[i], &temperature);
            
            if (sensors[i] == NULL) {
                continue;
            }

            if (result == DS1820_ERR_OK) {
                printf("%.2f\n", temperature);
            }

            else if (result == DS1820_ERR_EMPTYBUS || 
                     result == DS1820_ERR_NOANSWER) {
                printf("not present.\n");
            }

            else if (result == DS1820_ERR_BADCRC) {
                printf("crc error.\n");
            }
            else {
                printf("unknown error.");
            }
        }

        puts("");

        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}
