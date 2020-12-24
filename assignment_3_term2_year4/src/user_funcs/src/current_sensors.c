/*******************************************************************************************************
Description: working with ADC
Developer: Litvinov Oleg
Comments: No
*******************************************************************************************************/

#include "stm32f10x_adc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

#define ADC_IRQ_HANDLER ADC1_2_IRQHandler

/***************************************************************************************************
Local data types
***************************************************************************************************/

typedef void ( *RccClockCmd )( uint32_t, FunctionalState ); 

/***************************************************************************************************
Local file variables
***************************************************************************************************/
const RccClockCmd adc_pin_rcc_cmd = RCC_APB2PeriphClockCmd;
const uint32_t adc_pin_rcc = RCC_APB2Periph_GPIOB;
//From STM32F100x datasheet we find that ADC pins are as alternate functions
//as follows: ADC1_IN0 – PA0 , ADC1_IN1 – PA1
GPIO_TypeDef * const adc_port = GPIOA;
const uint16_t adc1_in0 = GPIO_Pin_0;
const uint16_t adc1_in1 = GPIO_Pin_1;

const RccClockCmd adc_rcc_cmd = RCC_APB2PeriphClockCmd;
const uint32_t adc_rcc = RCC_APB2Periph_ADC1;
ADC_TypeDef * const adc = ADC1;
const uint8_t adc_sample_time = ADC_SampleTime_239Cycles5;

const IRQn_Type adc_irq_type = ADC1_2_IRQn;
const uint8_t adc_irq_prio = 2;

uint16_t adcConvertedValue[2];
uint16_t channel = 0;


/**************************************************************************************************
Description: ADC initialization
Arguments: No
Return: No
Comments: for processing values from a current sensor
**************************************************************************************************/
void adc_init( void )
{
    adc_pin_rcc_cmd(adc_pin_rcc, ENABLE);//supply power on ADC pin

    //Configure ADC pins (PA0 -> Channel 0 to PA1 -> Channel 1) as 
	  //analog inputs for ADC
    GPIO_InitTypeDef gpioInitStructure;
    gpioInitStructure.GPIO_Pin = adc1_in0 | adc1_in1;
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
    adcInitStruct.ADC_NbrOfChannel = 2;
    adcInitStruct.ADC_ScanConvMode = ENABLE;
    ADC_Init( adc, &adcInitStruct );
    
    
		//configure ADC conversion channel
		//they will be handled in cycle:1-2-1-2-...
    ADC_RegularChannelConfig( adc, ADC_Channel_0, 1, adc_sample_time );
		ADC_RegularChannelConfig( adc, ADC_Channel_1, 2, adc_sample_time );
    
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
        
        adcConvertedValue[channel] = ADC_GetConversionValue( adc );
				
				channel++; 
				channel = channel % 2;
    }
}
