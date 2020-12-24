/***************************************************************************************************
Описание: Выдача напряжения на обмотки двигателя постоянного тока
Разработчик: Литвинов Олег
****************************************************************************************************/
#include "project_config.h"
#include "motor_voltage.h"

/***************************************************************************************************
Локальные дефайны
***************************************************************************************************/
// Частота генерации ШИМа = SystemCoreClock / PRESCALER / PERIOD = 72МГц /(72 * 1000) = 1 КГц
#define PERIOD 1000
#define PRESCALER (SystemCoreClock / 1000000)   // Частота счета таймера будет 1 МГц
#define MAX_PWM 1000

/***************************************************************************************************
Описание: Инициализация пинов под ШИМ
Аргументы: Нет
Возврат:   Нет
Замечания: Нет
***************************************************************************************************/
void initPWM(void) 
{   
    GPIO_InitTypeDef port;
    TIM_TimeBaseInitTypeDef timer;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); // Тактирование порта A
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);   // Тактирование TIM1
     
    //Настраиваем PA8 PA9 PA10 на генерацию ШИМ
    GPIO_StructInit(&port);
    port.GPIO_Mode = GPIO_Mode_AF_PP;
    port.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
    port.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &port);        
    
    TIM_TimeBaseStructInit(&timer);  
    
    //Настройка TIM1 на частоту 200Гц и направлением счета на возрастание
    timer.TIM_Prescaler = PRESCALER - 1;
    timer.TIM_Period = PERIOD - 1;
    timer.TIM_ClockDivision = 0;
    timer.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM1, &timer);  
    
    //Структура для канала TIM1
    TIM_OCInitTypeDef timerPWM;       
    
    TIM_OCStructInit(&timerPWM);
    
    timerPWM.TIM_OCMode = TIM_OCMode_PWM1;
    timerPWM.TIM_OutputState = TIM_OutputState_Enable;
    timerPWM.TIM_Pulse = 0;
    TIM_OC1Init(TIM1, &timerPWM);    // Настройка первого канала таймера
    TIM_OC2Init(TIM1, &timerPWM);    // Настройка второго канала таймера
    TIM_OC3Init(TIM1, &timerPWM);    // Настройка третьего канала таймера

    TIM_CtrlPWMOutputs(TIM1, ENABLE);
    TIM_Cmd(TIM1, ENABLE);           //Запуск TIM1
}

/***************************************************************************************************
Описание: Преобразование значения из одного диапазона в другой  
Аргументы:
x - значение, которое преобразуем
in_min/in_max  -диапазон, из которого преобразуем
out_min/out_max  -диапазон, в который преобразуем
Возврат:   Нет
Замечания: для генерации ШИМа на двигателе в диапазоне от -1000 до 1000 
***************************************************************************************************/
int32_t map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/***************************************************************************************************
Описание: Выдача напряжений обеих полярностей на обмотки двигателя  
Аргументы: pulseWidths - коэффициент заполнения ШИМа 
Возврат:   Нет
Замечания: для генерации ШИМа на двигателе в диапазоне от 0 до 1000 
***************************************************************************************************/
void setVoltage(int32_t voltageA, int32_t voltageB, int32_t voltageC)
{    
    int32_t pulseWidthsPhaseA = voltageA;
    int32_t pulseWidthsPhaseB = voltageB;
    int32_t pulseWidthsPhaseC = voltageC;

    pulseWidthsPhaseA = ((pulseWidthsPhaseA <= MAX_PWM) ? pulseWidthsPhaseA : MAX_PWM);
    // Подача напряжения на фазу А
    TIM_SetCompare1 (TIM1, pulseWidthsPhaseA);

    pulseWidthsPhaseB = ((pulseWidthsPhaseB <= MAX_PWM) ? pulseWidthsPhaseB : MAX_PWM);
    // Подача напряжения на фазу B
    TIM_SetCompare2 (TIM1, pulseWidthsPhaseB);

    pulseWidthsPhaseC = ((pulseWidthsPhaseC <= MAX_PWM) ? pulseWidthsPhaseC : MAX_PWM);
    // Подача напряжения на фазу C
    TIM_SetCompare3 (TIM1, pulseWidthsPhaseC);
}
