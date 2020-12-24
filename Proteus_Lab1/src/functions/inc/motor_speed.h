#pragma once

#include <stdint.h>

//Инициализация преобразователя
void adc_init( void );
//Старт преобразования преобразователя
void adc_startConvertion( void );
//Получить текущее значение скорости с пина PB0
int16_t motor_speed_getSpeed( void );