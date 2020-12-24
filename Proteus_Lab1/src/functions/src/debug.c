/*******************************************************************************************************
Description: working with USART
Developer: Litvinov Oleg
Comments: No
*******************************************************************************************************/
#include "stm32f10x_usart.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_dma.h"
#include "misc.h"

#include "stdbool.h"
#include "string.h"

#include "systick.h"
#include "debug.h"
#include "motor_speed.h"
/***************************************************************************************************
Local defines
***************************************************************************************************/
#define PC_LINK_UART USART1
#define PC_LINK_UART_PORT GPIOA
#define PC_LINK_UART_TX_PIN GPIO_Pin_9
#define PC_LINK_UART_RX_PIN GPIO_Pin_10

#define PC_LINK_UART_BR 115200

#define PC_LINK_UART_RCC_CMD(x) RCC_APB2PeriphClockCmd( RCC_APB2Periph_USART1, x)
#define PC_LINK_UART_PORT_RCC_CMD(x) RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA, x)

#define RX_BUF_SIZE 10

#define MAX_SPEED 2047
#define MIN_SPEED -2048

/***************************************************************************************************
Local function prototypes
***************************************************************************************************/
void initUartPins( void );
void clear_rx_buffer( void );
int16_t strToInt( char *s );

/***************************************************************************************************
Local file variables
***************************************************************************************************/
volatile bool rx_flag_end_line = false;
volatile bool startNewTask = false;
volatile uint16_t RXi;
volatile char RXc;
char RX_BUF[RX_BUF_SIZE] = {'\0'};

char sendSpeedStr[] = "+0000";
int16_t targetSpeed = 0;

/**************************************************************************************************
Description: Initializing pins to TX and RX mode
Arguments: No
Return: No
Comments: No
***************************************************************************************************/
void initUartPins( void )
{
    PC_LINK_UART_PORT_RCC_CMD(ENABLE);
    
    GPIO_InitTypeDef gpioInitStruct;
    
    gpioInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    gpioInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    gpioInitStruct.GPIO_Pin = PC_LINK_UART_TX_PIN;
    
    GPIO_Init( PC_LINK_UART_PORT, &gpioInitStruct );
    
    gpioInitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    gpioInitStruct.GPIO_Pin = PC_LINK_UART_RX_PIN;
    
    GPIO_Init( PC_LINK_UART_PORT, &gpioInitStruct );
}

/**************************************************************************************************
Description: USART1 initialization
Arguments: No
Return: No
Comments: No
***************************************************************************************************/
void uart_init( void )
{
    PC_LINK_UART_RCC_CMD(ENABLE);
    
    initUartPins();
    
    USART_InitTypeDef uartInitStruct;//USART configuring
    
    USART_StructInit( &uartInitStruct );
    uartInitStruct.USART_BaudRate = PC_LINK_UART_BR;
    uartInitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    
    USART_Init( PC_LINK_UART, &uartInitStruct );
    
    //set the priority of USART interruptions 
    NVIC_InitTypeDef nvicInitStruct;//
    
    nvicInitStruct.NVIC_IRQChannel = USART1_IRQn;
    nvicInitStruct.NVIC_IRQChannelCmd = ENABLE;
    nvicInitStruct.NVIC_IRQChannelPreemptionPriority = 3;
    nvicInitStruct.NVIC_IRQChannelSubPriority = 0;
    
    NVIC_Init( &nvicInitStruct );
    
    // run USART
    USART_Cmd( PC_LINK_UART, ENABLE );
    USART_ITConfig( PC_LINK_UART, USART_IT_RXNE, ENABLE);
    
    // connecting DMA to USART1
    USART_DMACmd( PC_LINK_UART, USART_DMAReq_Tx, ENABLE );
    
    // run DMA
    RCC_AHBPeriphClockCmd( RCC_AHBPeriph_DMA1, ENABLE );
}


/**************************************************************************************************
Description: transmitting from UART to PC
Arguments:
	buf: symbols array
	len: array length
Return: No
Comments: No
***************************************************************************************************/
void sendToPC( char* buf, uint32_t len )
{
	DMA_DeInit( DMA1_Channel4 );
	
	DMA_InitTypeDef dmaInitStruct;
	DMA_StructInit( &dmaInitStruct );
	dmaInitStruct.DMA_BufferSize = len;
	dmaInitStruct.DMA_MemoryBaseAddr = (uint32_t) buf;
	dmaInitStruct.DMA_DIR = DMA_DIR_PeripheralDST;
	dmaInitStruct.DMA_M2M = DMA_M2M_Disable;
	dmaInitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	dmaInitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
	dmaInitStruct.DMA_Mode = DMA_Mode_Normal;
	dmaInitStruct.DMA_PeripheralBaseAddr = (uint32_t) &(PC_LINK_UART->DR);
	dmaInitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	dmaInitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_Init( DMA1_Channel4, &dmaInitStruct );
	
	DMA_Cmd( DMA1_Channel4, ENABLE );
}

/**************************************************************************************************
Description: buffer clearing
Arguments: No
Return: No
Comments: No
***************************************************************************************************/
void clear_rx_buffer( void ) 
{
    for (RXi=0; RXi < RX_BUF_SIZE; RXi++)
    {
        RX_BUF[RXi] = '\0';
    }
    RXi = 0;
}

 
/**************************************************************************************************
Description: reading end of message receiving flag
Arguments: No
Return: No
Comments: No
***************************************************************************************************/
bool check_rx_flag ( void )
{
    return rx_flag_end_line;
}

/**************************************************************************************************
Description: reset end of message receiving flag
Arguments: No
Return: No
Comments: No
***************************************************************************************************/
void reset_rx_flag ( void )
{
    rx_flag_end_line = false;
}

 /**************************************************************************************************
Description: string to number convertion
Arguments:
	s: string to be converted
Return: No
Comments: No
***************************************************************************************************/
int16_t strToInt( char *s )
{
    int16_t temp = 0;
    bool isNegative = false;
    
    if ( s[0] == '-' )
    {
        isNegative = true;
    }
    else if ( s[0] != '+')
    {
        return targetSpeed;  
    }
    
    uint8_t i = 1;
    while ( s[i] >= '0' && s[i] <= '9' )
    {
        temp += s[i] - '0';
        temp = temp * 10;
        i++;
    }
    temp = temp / 10;
    
    if ( isNegative )
    {
        temp *= -1;
    }
    return temp;
}

/**************************************************************************************************
Description: check correctness of target speed
Arguments: No
Return: No
Comments: No
***************************************************************************************************/
void setTargetSpeed ( void )
{   
    int32_t task;
    task = strToInt( (char*)RX_BUF );
    
    if ( task >= MIN_SPEED && task <= MAX_SPEED)
    {
        targetSpeed = task;
    }
}


/**************************************************************************************************
Description: send current speed value to PC
Arguments: 
	speed: speed value to be transmitted
Return: No
Comments: No
***************************************************************************************************/
void sendSpeed( int16_t speed )
{   
    if ( speed < 0 ) 
    { 
        sendSpeedStr[0] = '-';
        speed = -speed;
    } 
    else 
    { 
        sendSpeedStr[0] = '+'; 
    } 
    for (uint8_t i = 4; i > 0; i--) 
    { 
        sendSpeedStr[i] = speed % 10 + '0';
        speed /= 10;
    }
    
    sendToPC(sendSpeedStr, sizeof(sendSpeedStr) - 1);
}             

/**************************************************************************************************
Description: get current speed value
Arguments: No
Return: No
Comments: No
***************************************************************************************************/
int16_t getCurTargetSpeed( void )
{   
    return targetSpeed;
}

/**************************************************************************************************
Description: user input
Arguments: No
Return: No
Comments: No
***************************************************************************************************/

void newTarget( void )
{
    if ( startNewTask )
    {
        startNewTask = false;
        sysTick_Stop();
        
        // waiting for the end of task input
        while ( check_rx_flag() == 0)
        {
            ;
        }
        
        reset_rx_flag();
        setTargetSpeed();
        
        sysTick_Start();
        clear_rx_buffer();
    }
}

 /**************************************************************************************************
Description: UART interruption
Arguments: No
Return: No
Comments: No
***************************************************************************************************/
void USART1_IRQHandler( void )
{
		// check for the event: if there is byte from the PC
    if ( USART_GetFlagStatus( PC_LINK_UART, USART_FLAG_RXNE ) )
    {       
        // Save received byte to RX_BUF
        RXc = USART_ReceiveData( USART1 );
        RX_BUF[RXi] = RXc;
        RXi++;

        // If \0 received (Enter pressed)
		if ( RXc != 13 )
		{
            if (RXc == 'b')
            {
                clear_rx_buffer();
                startNewTask = true;
            }
            if (RXi > RX_BUF_SIZE) 
            {
                clear_rx_buffer();
            }       
		}
        else 
        {
            rx_flag_end_line = true;
        } 
        // Echo - print in the terminal input characters
        USART_SendData(USART1, RXc);
    }
} 
