/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define BUZZER_PIN              (1U << 0)   /* PB0 */
#define LED_RED_PIN             (1U << 1)   /* PB1 */
#define LED_YELLOW_PIN          (1U << 2)   /* PB2 */

#define WARNING_THRESHOLD_ADC   1638U       /* about 2.0V if 0-5V maps to 0-4095 */
#define ALARM_THRESHOLD_ADC     2866U       /* about 3.5V if 0-5V maps to 0-4095 */
#define ADC_TIMEOUT             100000U
#define LOOP_DELAY              30000U
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void Alarm_GPIO_Init(void);
static void Alarm_ADC_Init(void);
static uint16_t Alarm_Read_ADC(uint8_t channel);
static void Alarm_Update(uint16_t smoke_adc, uint16_t gas_adc);
static void Alarm_Delay(volatile uint32_t count);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void Alarm_Delay(volatile uint32_t count)
{
  while (count--) {
    __asm volatile ("nop");
  }
}

static void Alarm_GPIO_Init(void)
{
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN;

  /* PA0/PA1: analog input for two potentiometers. */
  GPIOA->CRL &= ~((0xFU << 0) | (0xFU << 4));

  /* PB0/PB1/PB2: 2MHz push-pull outputs. */
  GPIOB->CRL &= ~((0xFU << 0) | (0xFU << 4) | (0xFU << 8));
  GPIOB->CRL |=  ((0x2U << 0) | (0x2U << 4) | (0x2U << 8));
  GPIOB->BRR = BUZZER_PIN | LED_RED_PIN | LED_YELLOW_PIN;
}

static void Alarm_ADC_Init(void)
{
  uint32_t timeout;

  RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

  /* ADC clock = PCLK2 / 2. With the default 8MHz HSI clock this is 4MHz. */
  RCC->CFGR &= ~(0x3U << 14);

  ADC1->CR1 = 0U;
  ADC1->CR2 = 0U;
  ADC1->SMPR2 &= ~((0x7U << 0) | (0x7U << 3));
  ADC1->SMPR2 |=  ((0x7U << 0) | (0x7U << 3));
  ADC1->CR2 |= ADC_CR2_EXTTRIG | ADC_CR2_EXTSEL;

  ADC1->CR2 |= ADC_CR2_ADON;
  Alarm_Delay(1000U);

  ADC1->CR2 |= ADC_CR2_RSTCAL;
  timeout = ADC_TIMEOUT;
  while (((ADC1->CR2 & ADC_CR2_RSTCAL) != 0U) && (timeout > 0U)) {
    timeout--;
  }

  ADC1->CR2 |= ADC_CR2_CAL;
  timeout = ADC_TIMEOUT;
  while (((ADC1->CR2 & ADC_CR2_CAL) != 0U) && (timeout > 0U)) {
    timeout--;
  }
}

static uint16_t Alarm_Read_ADC(uint8_t channel)
{
  uint32_t timeout = ADC_TIMEOUT;

  ADC1->SQR1 = 0U;
  ADC1->SQR3 = channel;
  ADC1->SR = 0U;
  ADC1->CR2 |= ADC_CR2_SWSTART;

  while (((ADC1->SR & ADC_SR_EOC) == 0U) && (timeout > 0U)) {
    timeout--;
  }

  if (timeout == 0U) {
    return 0U;
  }

  return (uint16_t)(ADC1->DR & 0x0FFFU);
}

static void Alarm_Update(uint16_t smoke_adc, uint16_t gas_adc)
{
  uint16_t level = smoke_adc;
  if (gas_adc > level) {
    level = gas_adc;
  }

  if (level >= ALARM_THRESHOLD_ADC) {
    GPIOB->BSRR = BUZZER_PIN | LED_RED_PIN;
    GPIOB->BRR = LED_YELLOW_PIN;
  } else if (level >= WARNING_THRESHOLD_ADC) {
    GPIOB->BSRR = LED_YELLOW_PIN;
    GPIOB->BRR = BUZZER_PIN | LED_RED_PIN;
  } else {
    GPIOB->BRR = BUZZER_PIN | LED_RED_PIN | LED_YELLOW_PIN;
  }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  /* USER CODE BEGIN 2 */
  Alarm_GPIO_Init();
  Alarm_ADC_Init();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    uint16_t smoke_adc = Alarm_Read_ADC(0U);
    uint16_t gas_adc = Alarm_Read_ADC(1U);

    Alarm_Update(smoke_adc, gas_adc);
    Alarm_Delay(LOOP_DELAY);

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
