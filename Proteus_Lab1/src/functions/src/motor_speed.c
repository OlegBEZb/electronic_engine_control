/*******************************************************************************************************
Description: working with ADC
Developer: Litvinov Oleg
Comments: No
*******************************************************************************************************/
#include "motor_speed.h"

#include "stm32f10x_adc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
/***************************************************************************************************
Local defines
***************************************************************************************************/

#define ADC_IRQ_HANDLER ADC1_2_IRQHandler
#define MAX_NEGATIVE_SPEED -2048

/***************************************************************************************************
Local data types
***************************************************************************************************/

typedef void ( *RccClockCmd )( uint32_t, FunctionalState ); 

/***************************************************************************************************
Local file variables
***************************************************************************************************/
const RccClockCmd adc_pin_rcc_cmd = RCC_APB2PeriphClockCmd;
const uint32_t adc_pin_rcc = RCC_APB2Periph_GPIOB;
GPIO_TypeDef * const adc_port = GPIOB;
const uint16_t adc_pin = GPIO_Pin_0;

const RccClockCmd adc_rcc_cmd = RCC_APB2PeriphClockCmd;
const uint32_t adc_rcc = RCC_APB2Periph_ADC1;
ADC_TypeDef * const adc = ADC1;
const uint8_t adc_channel = ADC_Channel_8;
const uint8_t adc_sample_time = ADC_SampleTime_239Cycles5;

const IRQn_Type adc_irq_type = ADC1_2_IRQn;
const uint8_t adc_irq_prio = 2;

static uint16_t adcConvertedValue;

/***************************************************************************************************
Global functions
***************************************************************************************************/

/**************************************************************************************************
Description: ADC initialization
Arguments: No
Return: No
Comments: for processing values from a potentiometer
**************************************************************************************************/
void adc_init( void )
{
    adc_pin_rcc_cmd(adc_pin_rcc, ENABLE);//supply power on ADC pin

    //ADC initialization
    GPIO_InitTypeDef gpioInitStructure;
    gpioInitStructure.GPIO_Pin = adc_pin;
    gpioInitStructure.GPIO_Mode = GPIO_Mode_AIN;
    gpioInitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init( adc_port, &gpioInitStructure );
    
	
    adc_rcc_cmd(adc_rcc, ENABLE);//supply power on ADC1 module
    
    //ADC1 module initializing
    ADC_InitTypeDef adcInitStruct;
    adcInitStruct.ADC_ContinuousConvMode = ENABLE;
    adcInitStruct.ADC_DataAlign = ADC_DataAlign_Right;
    adcInitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    adcInitStruct.ADC_Mode = ADC_Mode_Independent;
    adcInitStruct.ADC_NbrOfChannel = 1;
    adcInitStruct.ADC_ScanConvMode = ENABLE;
    ADC_Init( adc, &adcInitStruct );
    
    
		//configure ADC conversion channel
    ADC_RegularChannelConfig( adc, adc_channel, 1, adc_sample_time );
    
    //configure ADC interruptions
    ADC_ITConfig( adc, ADC_IT_EOC, ENABLE );
    
    __disable_irq();
    
    //set the priority of ADC interruptions 
    NVIC_SetPriority(adc_irq_type, adc_irq_prio);
    NVIC_EnableIRQ( adc_irq_type );
    
    
    ADC_Cmd(adc, ENABLE);//start ADC

    __enable_irq();
}


/**************************************************************************************************
Description: start convertion
Arguments: No
Return: data from ADC
Comments: No
**************************************************************************************************/
void adc_startConvertion( void )
{    
    ADC_SoftwareStartConvCmd( adc, ENABLE );
}

/**************************************************************************************************
Description: read conversion result
Arguments: No
Return: data from ADC
Comments: No
**************************************************************************************************/
int16_t motor_speed_getSpeed( void )
{
    return adcConvertedValue + MAX_NEGATIVE_SPEED;
}

/**************************************************************************************************
Description: ADC interruptions
Arguments: No
Return: No
Comments: read converted value and start the next conversion
**************************************************************************************************/
void ADC_IRQ_HANDLER()
{
    if( ADC_GetITStatus( adc, ADC_IT_EOC ) )
    {
        ADC_ClearITPendingBit(adc, ADC_IT_EOC);//clearing interrupion flag
        
        adcConvertedValue = ADC_GetConversionValue( adc );
    }
}
