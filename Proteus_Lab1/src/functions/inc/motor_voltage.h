#pragma once

#include <stdint.h>
//Инициализация ШИМА для TIM1 CH1 и PA8 + PA11 
//используется для подачи обратного напряжения(V-)
void pwm_init( uint32_t prescaler );
//Установка заполнения ШИМа для выходного напряжения на пине PA8
//Принимает значения от -1000 до 1000 (при отрицательном напряжении
//выдает 1 на порт PA11
void motor_voltage_setVoltage( int16_t );