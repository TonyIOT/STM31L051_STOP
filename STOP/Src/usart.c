#include "usart.h"
#include "stdio.h"

UART_HandleTypeDef huart1;

uint8_t uart1RxBuffer[UART1BUFMAX];
uint16_t uart1Count = 0;

#if 1
#pragma import(__use_no_semihosting)
//标准库需要的支持函数
struct __FILE
{
    int handle;

};

FILE __stdout;
//定义_sys_exit()以避免使用半主机模式
void _sys_exit(int x)
{
    x = x;
}
//重定义fputc函数
int fputc(int ch, FILE *f)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xffff);
    return ch;
}
#endif


void uart1Init(uint32_t bound)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = bound;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_RXOVERRUNDISABLE_INIT | UART_ADVFEATURE_DMADISABLEONERROR_INIT;
    huart1.AdvancedInit.OverrunDisable = UART_ADVFEATURE_OVERRUN_DISABLE;
    huart1.AdvancedInit.DMADisableonRxError = UART_ADVFEATURE_DMA_DISABLEONRXERROR;
    HAL_UART_Init(&huart1);
	
    HAL_UART_Receive_IT(&huart1, uart1RxBuffer, 1);
}


void USART1_IRQHandler(void)
{
    uint8_t res;

    if ((__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE) != RESET))
    {
        res = USART1->RDR;
        uart1RxBuffer[uart1Count++] = res;

        if (uart1Count > (UART1BUFMAX - 1))
        {
            uart1Count = 0;

        }
    }
}
