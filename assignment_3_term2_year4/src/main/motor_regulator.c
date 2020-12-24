/***************************************************************************************************
Описание: Файл для управления двигателем при помощи регуляторов
Разработчик: Литвинов Олег 
Заметки:
***************************************************************************************************/
#include "motor_regulator.h"   
#include <stdlib.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_tim.h>


/***************************************************************************************************
Локальные дефайны
***************************************************************************************************/
#define MAX_VALUE_PI 1000
#define MAX_VALUE_PID 4000

/***************************************************************************************************
Локальные типы данных
***************************************************************************************************/

/***************************************************************************************************
Локальные переменные файла
***************************************************************************************************/
// Коэффициенты усиления ПИД-регулятора
volatile float pidP = 10;
volatile float pidI = 0.5;
volatile float pidD = 0.01;

static int32_t prevErrPID = 0;
static int32_t accumErrPID = 0;

// Коэффициенты усиления ПИ-регулятора
volatile float piP = 1;
volatile float piI = 0.01;
static int32_t piIntTermD = 0;
static int32_t piIntTermQ = 0;


/***************************************************************************************************
Глобальные функции
***************************************************************************************************/

/***************************************************************************************************
Описание: ПИД-регулятор
Аргументы: err - ошибка управления
Возврат: Величина управляющего сигнала
Замечания: 
***************************************************************************************************/
int32_t PID_controller(int32_t err)
{
    int32_t impact;
    
    accumErrPID += pidI * err;
    impact = pidP * err;
    impact += accumErrPID;
    impact += pidD * (err - prevErrPID);

    prevErrPID = err;

    return impact;
}

/***************************************************************************************************
Описание: ПИ-регулятор тока по оси d
Аргументы: err - ошибка управления
Возврат: Величина управляющего сигнала
Замечания: 
***************************************************************************************************/
int32_t PI_controller_X_axis(int32_t err)
{
    int32_t impact;
    piIntTermD += piI * err;
    impact = piP * err;
    impact += piIntTermD;

    return impact;
}

/***************************************************************************************************
Описание: ПИ-регулятор тока по оси q
Аргументы: err - ошибка управления
Возврат: Величина управляющего сигнала
Замечания: 
***************************************************************************************************/
int32_t PI_controller_Y_axis(int32_t err)
{
    int32_t impact;
    piIntTermQ += piI * err;
    impact = piP * err;
    impact += piIntTermQ;

    return impact;
}