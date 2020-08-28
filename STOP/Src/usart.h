#ifndef __UASRT_H
#define __USART_H
#include "stm32l0xx_hal.h"

#define UART1BUFMAX 100

extern UART_HandleTypeDef huart1;

void uart1Init(uint32_t bound);

#endif
