/*******************************************************************************************************
Description: File of setting the voltage on the motor
Developer: Litvinov Oleg
Comments: No
*******************************************************************************************************/
#include "motor_voltage.h"

#include "stdlib.h"
#include "misc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"

/***************************************************************************************************
Local defines
***************************************************************************************************/
#define PERIOD 1000
#define INIT_PULSE_WIDTH 500

/**************************************************************************************************
Description: PWM pins initialization
Arguments: No
Return: No
Comments: No
**************************************************************************************************/
void pwm_init( uint32_t prescaler ) 
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//supply power on A port
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);//supply power on TIM1
    
		//Connecting motor V+ with PA.8 pin
	  //PA.8 pin connected with PWM
    GPIO_InitTypeDef Vplus; 
    Vplus.GPIO_Mode = GPIO_Mode_AF_PP;
    Vplus.GPIO_Pin = GPIO_Pin_8;
    Vplus.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &Vplus);
    
	
    TIM_TimeBaseInitTypeDef timer1;
    TIM_TimeBaseStructInit(&timer1);// fill the parameters by default
    timer1.TIM_Prescaler = prescaler - 1;
    timer1.TIM_CounterMode = TIM_CounterMode_Up;
    timer1.TIM_Period = PERIOD - 1;
    TIM_TimeBaseInit(TIM1, &timer1);
  
	
    TIM_OCInitTypeDef chTimer1;//TIM1 on the 3rd channel
    TIM_OCStructInit(&chTimer1);
    chTimer1.TIM_OCMode = TIM_OCMode_PWM1;
    chTimer1.TIM_OutputState = TIM_OutputState_Enable;
    chTimer1.TIM_Pulse = INIT_PULSE_WIDTH;
    TIM_OC1Init(TIM1, &chTimer1);
    
    //Connecting motor V- with PA.11 pin
    GPIOA->CRH &= ~(0xF000);//PA11 GP Output PUSH-PULL
    GPIOA->CRH |= 0x3000;
    
    
    TIM_CtrlPWMOutputs(TIM1, ENABLE);//start TIM1
    TIM_Cmd(TIM1, ENABLE);
}
/**************************************************************************************************
Description:  Value transformation from one interval to another
Arguments:
	value: value to be transformed
	firstMaxValue: higher dorder of first interval
	secondMaxValue: higher dorder of second interval
Return: Value in second interval
Comments: No
**************************************************************************************************/
int16_t conversion( int16_t value, int16_t firstMaxValue, int16_t secondMaxValue )
{
    if ( value > ( firstMaxValue ) / 2 )
    {
        return secondMaxValue / 2;
    }
    else if ( value <( -firstMaxValue - 1 ) / 2 )
    {
        return ( -secondMaxValue ) / 2;
    }
    return ( value * secondMaxValue / firstMaxValue );
}

/**************************************************************************************************
Description: send duty of the PWM to set voltage
Arguments:
	speed: motor speed in -2028 to 2047 interval
Return: No
Comments: new voltage equals VCC * voltage / PERIOD
**************************************************************************************************/

void motor_voltage_setVoltage( int16_t speed )
{
    int16_t voltage = conversion( speed, 4095, PERIOD * 2 );
    if ( voltage > 0 )  
    {
        GPIO_ResetBits(GPIOA, GPIO_Pin_11);
        TIM1->CCR1 = voltage;
    }
    else
    {
        GPIO_SetBits(GPIOA, GPIO_Pin_11);
        TIM1->CCR1 = PERIOD + voltage;
    }
}

