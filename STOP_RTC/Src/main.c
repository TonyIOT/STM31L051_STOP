#include "main.h"

/* RTC handler declaration */
RTC_HandleTypeDef RTCHandle;

void systemClockConfig(void);
static void gpioInit(void);
static void system_config_before_stop(void);
static void clock_config_after_stop(void);
static void stop_rtc_config(uint32_t sleep);
static void enter_stop_rtc_mode(void);

int main(void)
{

    HAL_Init();
    systemClockConfig();
    gpioInit();

    while (1)
    {
        LED_SET(0); //点亮LED
        HAL_Delay(5000);
        enter_stop_rtc_mode();//进入stop whit rtc mode
    }
}

static void enter_stop_rtc_mode(void)
{
    //1. 配置stop之前各外设
    system_config_before_stop();

    //2.设置rtc唤醒时间20s
    stop_rtc_config(20);

    //3. 进入stop
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

    //3.退出stop后恢复时钟
    clock_config_after_stop();

	//4.恢复各外设
    gpioInit();
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
    ret = HAL_RCC_OscConfig(&RCC_OscInitStructure);
    if (ret != HAL_OK) while (1);

    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
       clocks dividers */
    RCC_ClkInitStructure.ClockType = RCC_CLOCKTYPE_SYSCLK;
    RCC_ClkInitStructure.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    ret = HAL_RCC_ClockConfig(&RCC_ClkInitStructure, FLASH_LATENCY_1);
    if (ret != HAL_OK) while (1);
}


static void stop_rtc_config(uint32_t sleep)
{
    /* Disable Wakeup Counter */
    HAL_RTCEx_DeactivateWakeUpTimer(&RTCHandle);

    /*## Setting the Wake up time */
    HAL_RTCEx_SetWakeUpTimer_IT(&RTCHandle, sleep, RTC_WAKEUPCLOCK_CK_SPRE_16BITS);
}

//进入stop之前外设设置
static void system_config_before_stop(void)
{
    HAL_StatusTypeDef ret = HAL_OK;

    GPIO_InitTypeDef GPIO_InitStructure;

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
    GPIO_InitStructure.Pin = GPIO_PIN_All;
    GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
    HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
    HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);


    /* Disable GPIOs clock */
    __HAL_RCC_GPIOA_CLK_DISABLE();
    __HAL_RCC_GPIOB_CLK_DISABLE();
    __HAL_RCC_GPIOC_CLK_DISABLE();



    /* Configure RTC */
    RTCHandle.Instance = RTC;
    RTCHandle.Init.HourFormat = RTC_HOURFORMAT_24;
    RTCHandle.Init.AsynchPrediv = RTC_ASYNCH_PREDIV;
    RTCHandle.Init.SynchPrediv = RTC_SYNCH_PREDIV;
    RTCHandle.Init.OutPut = RTC_OUTPUT_DISABLE;
    RTCHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    RTCHandle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;

    ret = HAL_RTC_Init(&RTCHandle);
    if (ret != HAL_OK) while (1);
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

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
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

}

