/* Declarations for beep.c */
#include "esp_system.h" //esp_err_t

esp_err_t beep_init(uint32_t pin_num0);
void beep(int freqhz, int lenms);
