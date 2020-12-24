/*******************************************************************************************************
Description: main file
Laboratory work №: 1
Subject: microcontrollers programming in robotics
Developer: Litvinov Oleg
Group: 43328/1

Options for target:

define: STM32F103xB, USE_FULL_ASSERT, USE_STDPERIPH_DRIVER, STM32F10X_MD, HSE_VALUE=8000000
include paths: .\src\cmsis;.\src;.\src\mcu_support_package\inc;.\src\spl\inc;.\src\spl;.\src\functions\inc;.\src\functions
misc controls: --feedback=unused
version c: c99
*******************************************************************************************************/

#include <stdint.h>
#include <stdbool.h>

#include "systick.h"
#include "debug.h"
#include "motor_speed.h"
#include "motor_voltage.h"
#include "control.h"

//#define KEIL
#define PROTEUS

#ifdef KEIL
const uint32_t tim1_prescaler = 72 * 10;//value used to divide the TIM clock
const uint32_t sysTick_1ms = 72 * 1000;
#endif

#ifdef PROTEUS
const uint32_t tim1_prescaler = 1;
const uint32_t sysTick_1ms = 100;
#endif
/***************************************************************************************************
Global functions
***************************************************************************************************/
void all_init( void ) 
{
    systick_init( sysTick_1ms );
    pwm_init( tim1_prescaler );
    adc_init();
    uart_init();
    adc_startConvertion();  
}

int main( void )
{
    all_init();
	
    while ( true )
    {
        if ( getCurrentTime() > 1000 )
        {
            int16_t speed = motor_speed_getSpeed();
            sendSpeed( speed );
            setToZeroCurrentTime();
        }
        
        newTarget();
        int16_t target = getCurTargetSpeed();
        
        if ( getCurrentTimeForPWM() > 100 )
        {
            int16_t impact = control_run( target );
            motor_voltage_setVoltage( impact );
            setToZeroCurrentTimeForPWM();
        }
    }
}


#ifdef  USE_FULL_ASSERT
/*
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
    /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    
    /* Infinite loop */
    while (1)
    {
    }
}
#endif
