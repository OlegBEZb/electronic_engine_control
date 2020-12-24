#pragma once

#include <stdint.h>

uint32_t getCurrentTime( void );
void setToZeroCurrentTime( void );

uint32_t getCurrentTimeForPWM( void );
void setToZeroCurrentTimeForPWM( void );
//инициализация систика
void systick_init( uint32_t ticks );
//задержка в миллисекундах(зависит от миикры)
void delay_ms( uint32_t delay );
//Запуск таймера SysTick
void sysTick_Start( void );
//Остановка таймера SysTick
void sysTick_Stop( void );
