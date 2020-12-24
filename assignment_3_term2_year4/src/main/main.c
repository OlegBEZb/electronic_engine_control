/***********************************************
Название работы: Управление током и скоростью двигателя переменного тока

Разработчик: Литвинов Олег

Техническое задание:
Написать программу на языке С согласно стандарту, описанному в файле. 
Программа должна обеспечивать следующий функционал:
•	Выдача напряжения на обмотки двигателя с обеспечением верного порядка коммутации
•	Чтение данных с датчиков тока
•	Чтение данных с датчика положения ротора
•	Управление током двигателя на основе ПИ-регулятора
•	Управление скоростью двигателя на основе ПИД-регулятора

Параметры установки:
•	Для синхронного привода: 
    Фаза двигателя А -> TIM1 channel1 
    Фаза двигателя B -> TIM1 channel2 
    Фаза двигателя C -> TIM1 channel3
•	Датчик тока фазы А -> ADC1 input0 
  Датчик тока фазы B -> ADC1 input1. 
  Смещение 1.65В
•	Датчик положения TIM2->CNT
•	Для определения типа двигателя надо зайти в отладку, в окне Command вбить 
is_pmsm 1 - синхронный двигатель, 0 - шаговый. В данной работе 1.
•	Разрешение датчика угла поворота определяется командой angle_sensor_resolution 
Ноль датчика совпадает с фазой А двигателя. В данной работе 1024.
***********************************************/

#pragma once

#include "adc.h"

#include "project_config.h"
#include "motor_voltage.h"
#include "motor_regulator.h"
#include "motor_revert.h"

#define MAX_CURRENT 2000
#define MAX_SPEED 10000

static int32_t sensorSpeed = 0; // Скорость, вычисленная с датчика положения

static int32_t sensorCurrentA = 0;
static int32_t sensorCurrentB = 0;
static int32_t sensorCurrentC = 0;

static int32_t signalVoltageA = 0;
static int32_t signalVoltageB = 0;
static int32_t signalVoltageC = 0;

static int32_t currentErrX = 0;
static int32_t currentErrY = 0;

static int32_t sensorCurrentX = 0; 
static int32_t sensorCurrentY = 0;

// Скорость задается в диапазоне -100...100 об/сек
volatile int32_t targetSpeed = -50; // Желаемое значение
static int32_t speedErr = 0;    // Ошибка по управлению скоростью

static int32_t signalVoltageX = 0;
static int32_t signalVoltageY = 0;

volatile int32_t currentTask = 0;

static int32_t angle = 0;

#define SYSTICK_PRESCALER 1000 
#define DELAY 1

/***************************************************************************************************
Локальные переменные файла
***************************************************************************************************/
volatile uint32_t timeStampMs = 0;

/***************************************************************************************************
Прототипы локальных функций
***************************************************************************************************/

void systickStart(void);
void systickStop(void);
bool delayMs(uint32_t delay);

/***************************************************************************************************
Описание: Временная задержка по SysTick
Аргументы: delay - время задержки в мс
Возврат: true - прошло заданное время, false - нет
Замечания: 
***************************************************************************************************/
bool delayMs(uint32_t delay)
{                                                                                    
    if (timeStampMs < delay)
    {
        return false;
    }
    else
    {
        timeStampMs = 0;
        return true;
    }
}

  /*************************************************************************************************
Описание: Остановка таймера SysTick
Аргументы: Нет
Возврат:   Нет
Замечания: 
***************************************************************************************************/
void systickStop(void)
{
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE;
}

/***************************************************************************************************
Описание: Запуск таймера SysTick
Аргументы: Нет
Возврат:   Нет
Замечания: 
***************************************************************************************************/
void systickStart(void)
{
    SysTick->CTRL |= SysTick_CTRL_ENABLE;
}

/***************************************************************************************************
Описание: Чтение значения с датчика положения ротора двигателя
Аргументы: Нет
Возврат:   Значение от 0 до 1024
Замечания: Значение с датчика положения считывается в TIM2->CNT
***************************************************************************************************/
uint16_t getAngle(void)
{
    return TIM2->CNT;
}

/***************************************************************************************************
Глобальные функции
***************************************************************************************************/

/***************************************************************************************************
Описание: Получение значения скорости двигателя
Аргументы: Нет
Возврат:   Значение скорости двигателя
Замечания: 
***************************************************************************************************/
int32_t getSpeed(int32_t currentAngle)
{ 
    static int32_t prevAngle = 0;
    float motorSpeed = 0;

    // Если ротор двигателя прошел через начальное положение
    if(abs(currentAngle - prevAngle) > 600)
    {
        // Если ротор двигателя прошел через начальное положение в положительном вращении
        if(currentAngle - prevAngle < -600)
        {
            motorSpeed = (float)(SYSTICK_PRESCALER / DELAY) * (1024 - prevAngle + currentAngle); 
        }
        // Если ротор двигателя прошел через начальное положение в отрицательном вращении
        else
        {
            motorSpeed = (float)(SYSTICK_PRESCALER / DELAY) * (-1024 - prevAngle + currentAngle);
        }
    }
    // Если ротор двигателя вращается себе спокойно
    else
    {
        motorSpeed = (float)(currentAngle - prevAngle) * (SYSTICK_PRESCALER / DELAY);
    }   
    motorSpeed /= 1024;  // Нормируем к об/сек

    prevAngle = currentAngle;

    return (int32_t)motorSpeed;
}

/***************************************************************************************************
Описание: Инициализация SysTick
Аргументы: Нет
Возврат:   Нет
Замечания: Смотреть описание SYSTICK_PRESCALER
***************************************************************************************************/
void systick_init (void)
{     
    SysTick_Config(SystemCoreClock/SYSTICK_PRESCALER); 
}

/***************************************************************************************************
Описание: Прерывания по Systick
Аргументы: Нет
Возврат:   Нет
Замечания: 
***************************************************************************************************/
void SysTick_Handler()
{
    timeStampMs++;
}

int main(void)
{   
    initPWM();
    adc_init();
    adc_startConvertion();
    systick_init();
    
    while(1)
    {
        // Управление производится через каждые DELAY мсек
        if(delayMs(DELAY) == true)
        {
            angle = getAngle(); // Получение угла положения ротора в градусах
            //задаётся мотор инитом
            
            // Обратная связь по току в диапазоне -2048...2047
            sensorCurrentA = getCurrentFromADC(0);       // Значение тока с датчика тока фазы А 
            sensorCurrentB = getCurrentFromADC(1);       // Значение тока с датчика тока фазы B
            sensorCurrentC = - sensorCurrentA - sensorCurrentB; // Значение тока с фазы C (Ia + Ib + Ic = 0)

            // Перевод тока из трехфазной СК статора в подвижную двуфазную СК ротора XY(DQ)
            transf_ABC_to_moving_XY(sensorCurrentA, sensorCurrentB, sensorCurrentC, angle); 
            // Получение проекций вектора тока в осях СК ротора XY(DQ)
            sensorCurrentX = getCurrentX();
            sensorCurrentY = getCurrentY();
            
            // Значение скорости с датчика на моторе
            sensorSpeed = getSpeed(angle);     
            // Ошибка управления по скорости
            speedErr = targetSpeed - sensorSpeed;
            // Величина по Y(Q) задающего входного тока сигнал с ПИДа (-2000...2000)
            // Y -задающий вектор
            // X - текущий вектор, т.к. сонаправлен с положением ротора. 
            // Текущий перпендикулярен задающему.
            // Для осуществления вращения мы хотим максимизировать ток по Y 
            // и минимизировать по X, т.к. ток по Х тормозит ротор
            currentTask = PID_controller(speedErr);
          
            // Ошибки по току
            currentErrY = currentTask - sensorCurrentY;
            currentErrX = - sensorCurrentX;

            // Получения сигнала управления с ПИ-регулятора
            signalVoltageX = PI_controller_X_axis(currentErrX);
            signalVoltageY = PI_controller_Y_axis(currentErrY);

            // Формирование задающего воздействия (напряжения): сначала перевод 
            // напряжения из XY(DQ) в Alpha-Beta (двухфахная статора), 
            // затем в ABC (трехфазная СК статора) 
            transf_XY_to_static_ABC(signalVoltageX, signalVoltageY, angle);

            signalVoltageA = getVoltageA();
            signalVoltageB = getVoltageB();
            signalVoltageC = getVoltageC();

            // Перевод отрицательных составляющих таким образом,
            // чтобы фазная разность оставалась прежней
            if(signalVoltageA < 0)
            {
                signalVoltageB -= signalVoltageA;
                signalVoltageC -= signalVoltageA;
                signalVoltageA = 0;
            }
            
            if(signalVoltageB < 0)
            {
                signalVoltageA -= signalVoltageB;
                signalVoltageC -= signalVoltageB;
                signalVoltageB = 0;
            }
            
            if(signalVoltageC < 0)
            {
                signalVoltageA -= signalVoltageC;
                signalVoltageB -= signalVoltageC;
                signalVoltageC = 0;
            }

            setVoltage(signalVoltageA, signalVoltageB, signalVoltageC);
        }
    }
}

// классический ассерт для STM32
#ifdef USE_FULL_ASSERT
    void assert_failed(uint8_t * file, uint32_t line)
    { 
        /* User can add his own implementation to report the file name and line number,
         ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
         
        (void)file;
        (void)line;

        while(1){};
    }
#endif
