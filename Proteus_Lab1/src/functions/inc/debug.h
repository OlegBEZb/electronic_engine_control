#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

void uart_init( void );

void sendToPC( char* buf, uint32_t len );

void newTarget( void );

void setTargetSpeed ( void );

void sendSpeed ( int16_t speed );

int16_t getCurTargetSpeed( void );

void clear_rx_buffer(void);

bool check_rx_flag( void );

void reset_rx_flag( void );

