/*******************************************************************************************************
Description: working with SysTick
Developer: Litvinov Oleg
Comments: No
*******************************************************************************************************/
#include <stdint.h>
#include <misc.h>

#include "systick.h"

/***************************************************************************************************
Local file variables
***************************************************************************************************/
volatile uint32_t timeStampMs = 0;
volatile uint32_t timeMsForPWM = 0;

/**************************************************************************************************
Description:  SysTick initialization
Arguments: No
Return: No
Comments: No
***************************************************************************************************/
void systick_init ( uint32_t ticks)
{    
    SysTick_Config( ticks );
}
/**************************************************************************************************
Description: Get current time
Arguments: No
Return: current time in ms
Comments: No 
***************************************************************************************************/
uint32_t getCurrentTime( void )
{
    return timeStampMs;
}
uint32_t getCurrentTimeForPWM( void )
{
    return timeMsForPWM;
}
/**************************************************************************************************
Description: set current time to zero
Arguments: No
Return: No
Comments: No
***************************************************************************************************/
void setToZeroCurrentTime( void )
{
    timeStampMs = 0;
}
void setToZeroCurrentTimeForPWM( void )
{
    timeMsForPWM = 0;
}
/**************************************************************************************************
Description: time delay using SysTick
Arguments: 
	delay: delay time in ms
Return: No
Comments: No 
***************************************************************************************************/

void delay_ms( uint32_t delay )
{
    uint32_t time = timeStampMs;
    while (timeStampMs < time + delay);
}

/**************************************************************************************************
Description: stop the SysTick timer
Arguments: No
Return: No
Comments: No 
***************************************************************************************************/

void sysTick_Stop( void )
{
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE;
}

/**************************************************************************************************
Description: start SysTick timer
Arguments: No
Return: No
Comments: No  
***************************************************************************************************/

void sysTick_Start( void )
{
    SysTick->CTRL |= SysTick_CTRL_ENABLE;
}

/**************************************************************************************************
Description: Systick interruptions
Arguments: No
Return: No
Comments: No  
***************************************************************************************************/

void SysTick_Handler()
{
    timeStampMs++;
    timeMsForPWM++;
}