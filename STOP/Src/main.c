#include "main.h"
#include "usart.h"


void systemClockConfig(void);
static void gpioInit(void);
static void system_config_before_stop(void);
static void clock_config_after_stop(void);
static void stop_exit_config(void);
static void enter_stop_mode(void);


int main(void)
{
    HAL_Init();
    systemClockConfig();
    gpioInit();
    uart1Init(115200);

    while (1)
    {
		LED_SET(0); //点亮LED
        HAL_Delay(5000);
	
		printf("try to enter stop mode...\r\n");		
		enter_stop_mode();		
		printf("exit from stop mode...\r\n");
		
    }
}

//进入stop之前外设设置
static void system_config_before_stop(void)
{
    GPIO_InitTypeDef GPIO_Initure;

    HAL_UART_DeInit(&huart1);
    /* Enable Ultra low power mode */
    HAL_PWREx_EnableUltraLowPower();

    /* Enable the fast wake up from Ultra low power mode */
    HAL_PWREx_EnableFastWakeUp();

    /* Select HSI as system clock source after Wake Up from Stop mode */
    __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_HSI);

    /* Enable GPIOs clock */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* Configure all GPIO port pins in Analog Input mode (floating input trigger OFF) */
    GPIO_Initure.Pin = GPIO_PIN_All;
    GPIO_Initure.Mode = GPIO_MODE_ANALOG;
    GPIO_Initure.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_Initure);
    HAL_GPIO_Init(GPIOB, &GPIO_Initure);
    HAL_GPIO_Init(GPIOC, &GPIO_Initure);

    /* Disable GPIOs clock */
    __HAL_RCC_GPIOA_CLK_DISABLE();
    __HAL_RCC_GPIOB_CLK_DISABLE();
    __HAL_RCC_GPIOC_CLK_DISABLE();

}

//退出stop后重新设置时钟
static void clock_config_after_stop(void)
{
    HAL_StatusTypeDef ret = HAL_OK;
    RCC_OscInitTypeDef RCC_OscInitStructure = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStructure = {0};

    /* Enable Power Control clock */
    __HAL_RCC_PWR_CLK_ENABLE();

    /* The voltage scaling allows optimizing the power consumption when the device is
       clocked below the maximum system frequency, to update the voltage scaling value
       regarding system frequency refer to product datasheet.  */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /* Get the Oscillators configuration according to the internal RCC registers */
    HAL_RCC_GetOscConfig(&RCC_OscInitStructure);

    /* After wake-up from STOP reconfigure the system clock: Enable HSI and PLL */
    RCC_OscInitStructure.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStructure.HSEState = RCC_HSE_OFF;
    RCC_OscInitStructure.HSIState = RCC_HSI_ON;
    RCC_OscInitStructure.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStructure.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStructure.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStructure.PLL.PLLMUL = RCC_PLLMUL_4;
    RCC_OscInitStructure.PLL.PLLDIV = RCC_PLLDIV_2;
    ret = HAL_RCC_OscConfig(&RCC_OscInitStructure); //初始化
    if (ret != HAL_OK) while (1);

    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
       clocks dividers */
    RCC_ClkInitStructure.ClockType = RCC_CLOCKTYPE_SYSCLK;
    RCC_ClkInitStructure.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    ret = HAL_RCC_ClockConfig(&RCC_ClkInitStructure, FLASH_LATENCY_1); //同时设置FLASH延时周期为1
    if (ret != HAL_OK) while (1);
}


//设置唤醒中断
static void stop_exit_config(void)
{
    GPIO_InitTypeDef GPIO_Initure = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_Initure.Pin = KEY_PIN;
    GPIO_Initure.Mode = GPIO_MODE_IT_RISING;
    GPIO_Initure.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(KEY_PORT, &GPIO_Initure);

    HAL_NVIC_SetPriority(EXTI2_3_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);

    GPIO_Initure.Pin = GPIO_PIN_10;
    GPIO_Initure.Mode = GPIO_MODE_IT_FALLING;
    GPIO_Initure.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_Initure);
    HAL_NVIC_SetPriority(EXTI4_15_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);

}


static void enter_stop_mode(void)
{
    //1. 配置stop之前各外设
    system_config_before_stop();

    //2.设置唤醒中断
    stop_exit_config();

    //3. 进入stop
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

    //3.退出stop后恢复各外设
    clock_config_after_stop();

	//4.恢复各外设
    gpioInit();
    uart1Init(115200);
}


static void gpioInit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /*Configure GPIO pin : PA1 */
    GPIO_InitStruct.Pin = LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);

    /*Configure GPIO pin : PA2 */
    GPIO_InitStruct.Pin = KEY_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(KEY_PORT, &GPIO_InitStruct);

}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == KEY_PIN)
    {
		
        /* Clear Wake Up Flag */
        __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

    }

    if (GPIO_Pin == GPIO_PIN_10)
    {
        /* Clear Wake Up Flag */
        __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

    }

}


void EXTI2_3_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(KEY_PIN);
}

void EXTI4_15_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10);
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (HSI)
  *            SYSCLK(Hz)                     = 32000000
  *            HCLK(Hz)                       = 32000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            HSI Frequency(Hz)              = 16000000
  *            PLL_MUL                        = 4
  *            PLL_DIV                        = 2
  *            Flash Latency(WS)              = 1
  *            Main regulator output voltage  = Scale1 mode
  * @param  None
  * @retval None
  */
void systemClockConfig(void)
{
    HAL_StatusTypeDef ret = HAL_OK;
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
    */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    /** Initializes the CPU, AHB and APB busses clocks
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLLMUL_4;
    RCC_OscInitStruct.PLL.PLLDIV = RCC_PLLDIV_2;
    ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);

    if (ret != HAL_OK) while (1);

    /** Initializes the CPU, AHB and APB busses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1);

    if (ret != HAL_OK) while (1);
}


