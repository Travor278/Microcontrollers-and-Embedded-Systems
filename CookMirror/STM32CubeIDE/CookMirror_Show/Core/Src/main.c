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
#define LCD_GPIO_PORT          GPIOB
#define LCD_RS_PIN             GPIO_PIN_10
#define LCD_E_PIN              GPIO_PIN_11
#define LCD_D4_PIN             GPIO_PIN_12
#define LCD_D5_PIN             GPIO_PIN_13
#define LCD_D6_PIN             GPIO_PIN_14
#define LCD_D7_PIN             GPIO_PIN_15

#define COMM_LED_GPIO_PORT     GPIOC
#define COMM_LED_PIN           GPIO_PIN_3

#define UART_BAUDRATE          9600U
#define UART_PCLK_HZ           8000000U
#define STATUS_PERIOD_MS       1500U
#define CONTRAST_UPDATE_MS     200U
#define CONTRAST_ADC_TIMEOUT   100000U
#define CONTRAST_ADC_CHANNEL   4U      /* PA4 / ADC1_IN4, tied to LCD VEE */
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
static void Show_GPIO_Init(void);
static void Show_ADC_Init(void);
static void Show_UART_Init(void);
static void LCD_Init(void);
static void LCD_Clear(void);
static void LCD_SetCursor(uint8_t row, uint8_t col);
static void LCD_Print(const char *text);
static void LCD_Command(uint8_t command);
static void LCD_Data(uint8_t data);
static void LCD_Write4(uint8_t value);
static void LCD_PulseEnable(void);
static void Show_RenderStep(void);
static void Show_UpdateContrastLine(void);
static void Show_SendStatus(void);
static void Show_HandleCommand(uint8_t byte);
static void Show_UART_WriteChar(USART_TypeDef *uart, char ch);
static void Show_UART_WriteString(USART_TypeDef *uart, const char *text);
static uint8_t Show_UART_ReadByte(USART_TypeDef *uart, uint8_t *byte);
static uint16_t Show_ReadContrastAdc(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static const char *const recipe_steps[] = {
  "Step 1/4: Heat pan",
  "Step 2/4: Add oil",
  "Step 3/4: Add food",
  "Step 4/4: Plate dish"
};

static uint8_t current_step = 0U;
static uint32_t last_status_tick = 0U;
static uint32_t last_contrast_tick = 0U;
static uint8_t last_contrast_percent = 255U;

static void Show_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_AFIO_CLK_ENABLE();

  HAL_GPIO_WritePin(LCD_GPIO_PORT,
                    LCD_RS_PIN | LCD_E_PIN | LCD_D4_PIN | LCD_D5_PIN |
                    LCD_D6_PIN | LCD_D7_PIN,
                    GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = LCD_RS_PIN | LCD_E_PIN | LCD_D4_PIN | LCD_D5_PIN |
                        LCD_D6_PIN | LCD_D7_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_GPIO_PORT, &GPIO_InitStruct);

  HAL_GPIO_WritePin(COMM_LED_GPIO_PORT, COMM_LED_PIN, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin = COMM_LED_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(COMM_LED_GPIO_PORT, &GPIO_InitStruct);

  /* PA4: analog input. Connect the LCD VEE/RV1 wiper net here. */
  GPIOA->CRL &= ~(0xFU << 16);

  /* PA2/PA9: USART TX alternate-function push-pull. PA3/PA10: RX input. */
  GPIOA->CRL &= ~((0xFU << 8) | (0xFU << 12));
  GPIOA->CRL |=  ((0xAU << 8) | (0x4U << 12));
  GPIOA->CRH &= ~((0xFU << 4) | (0xFU << 8));
  GPIOA->CRH |=  ((0xAU << 4) | (0x4U << 8));
}

static void Show_ADC_Init(void)
{
  uint32_t timeout;

  __HAL_RCC_ADC1_CLK_ENABLE();

  /* ADC clock = PCLK2 / 2. With HSI at 8MHz this keeps ADC1 at 4MHz. */
  RCC->CFGR &= ~RCC_CFGR_ADCPRE;

  ADC1->CR1 = 0U;
  ADC1->CR2 = 0U;
  ADC1->SMPR2 &= ~(0x7U << (CONTRAST_ADC_CHANNEL * 3U));
  ADC1->SMPR2 |=  (0x7U << (CONTRAST_ADC_CHANNEL * 3U));
  ADC1->CR2 |= ADC_CR2_EXTTRIG | ADC_CR2_EXTSEL;

  ADC1->CR2 |= ADC_CR2_ADON;
  HAL_Delay(1U);

  ADC1->CR2 |= ADC_CR2_RSTCAL;
  timeout = CONTRAST_ADC_TIMEOUT;
  while (((ADC1->CR2 & ADC_CR2_RSTCAL) != 0U) && (timeout > 0U)) {
    timeout--;
  }

  ADC1->CR2 |= ADC_CR2_CAL;
  timeout = CONTRAST_ADC_TIMEOUT;
  while (((ADC1->CR2 & ADC_CR2_CAL) != 0U) && (timeout > 0U)) {
    timeout--;
  }
}

static void Show_UART_Init(void)
{
  uint32_t brr = (UART_PCLK_HZ + (UART_BAUDRATE / 2U)) / UART_BAUDRATE;

  __HAL_RCC_USART1_CLK_ENABLE();
  __HAL_RCC_USART2_CLK_ENABLE();

  USART1->BRR = brr;
  USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;

  USART2->BRR = brr;
  USART2->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

static void LCD_PulseEnable(void)
{
  HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_E_PIN, GPIO_PIN_SET);
  HAL_Delay(1U);
  HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_E_PIN, GPIO_PIN_RESET);
  HAL_Delay(1U);
}

static void LCD_Write4(uint8_t value)
{
  HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_D4_PIN, (value & 0x01U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_D5_PIN, (value & 0x02U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_D6_PIN, (value & 0x04U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_D7_PIN, (value & 0x08U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  LCD_PulseEnable();
}

static void LCD_Command(uint8_t command)
{
  HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_RS_PIN, GPIO_PIN_RESET);
  LCD_Write4(command >> 4);
  LCD_Write4(command & 0x0FU);
  HAL_Delay(2U);
}

static void LCD_Data(uint8_t data)
{
  HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_RS_PIN, GPIO_PIN_SET);
  LCD_Write4(data >> 4);
  LCD_Write4(data & 0x0FU);
  HAL_Delay(1U);
}

static void LCD_Init(void)
{
  HAL_Delay(50U);
  HAL_GPIO_WritePin(LCD_GPIO_PORT, LCD_RS_PIN | LCD_E_PIN, GPIO_PIN_RESET);

  LCD_Write4(0x03U);
  HAL_Delay(5U);
  LCD_Write4(0x03U);
  HAL_Delay(1U);
  LCD_Write4(0x03U);
  LCD_Write4(0x02U);

  LCD_Command(0x28U);
  LCD_Command(0x0CU);
  LCD_Command(0x06U);
  LCD_Clear();
}

static void LCD_Clear(void)
{
  LCD_Command(0x01U);
  HAL_Delay(2U);
}

static void LCD_SetCursor(uint8_t row, uint8_t col)
{
  static const uint8_t row_addr[] = {0x00U, 0x40U, 0x14U, 0x54U};
  if (row > 3U) {
    row = 3U;
  }
  LCD_Command((uint8_t)(0x80U | row_addr[row] | col));
}

static void LCD_Print(const char *text)
{
  while (*text != '\0') {
    LCD_Data((uint8_t)*text);
    text++;
  }
}

static void Show_RenderStep(void)
{
  LCD_Clear();
  LCD_SetCursor(0U, 0U);
  LCD_Print("CookMirror Demo");
  LCD_SetCursor(1U, 0U);
  LCD_Print(recipe_steps[current_step]);
  LCD_SetCursor(2U, 0U);
  LCD_Print("UART: send N");
  last_contrast_percent = 255U;
  Show_UpdateContrastLine();
}

static void Show_UART_WriteChar(USART_TypeDef *uart, char ch)
{
  while ((uart->SR & USART_SR_TXE) == 0U) {
  }
  uart->DR = (uint16_t)ch;
}

static void Show_UART_WriteString(USART_TypeDef *uart, const char *text)
{
  while (*text != '\0') {
    Show_UART_WriteChar(uart, *text);
    text++;
  }
}

static uint8_t Show_UART_ReadByte(USART_TypeDef *uart, uint8_t *byte)
{
  if ((uart->SR & USART_SR_RXNE) == 0U) {
    return 0U;
  }

  *byte = (uint8_t)(uart->DR & 0xFFU);
  return 1U;
}

static uint16_t Show_ReadContrastAdc(void)
{
  uint32_t timeout = CONTRAST_ADC_TIMEOUT;

  ADC1->SQR1 = 0U;
  ADC1->SQR3 = CONTRAST_ADC_CHANNEL;
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

static void Show_UpdateContrastLine(void)
{
  uint16_t adc_value = Show_ReadContrastAdc();
  uint8_t percent = (uint8_t)(((uint32_t)adc_value * 100U + 2047U) / 4095U);
  uint8_t filled = (uint8_t)(((uint32_t)percent * 5U + 50U) / 100U);
  uint8_t i;

  if (percent > 100U) {
    percent = 100U;
  }
  if (filled > 5U) {
    filled = 5U;
  }
  if (percent == last_contrast_percent) {
    return;
  }

  last_contrast_percent = percent;

  LCD_SetCursor(3U, 0U);
  LCD_Print("Contrast ");
  LCD_Data((percent >= 100U) ? '1' : ' ');
  LCD_Data((uint8_t)('0' + ((percent / 10U) % 10U)));
  LCD_Data((uint8_t)('0' + (percent % 10U)));
  LCD_Data('%');
  LCD_Data(' ');
  for (i = 0U; i < 5U; i++) {
    LCD_Data((i < filled) ? '#' : '-');
  }
  LCD_Data(' ');
}

static void Show_HandleCommand(uint8_t byte)
{
  if ((byte == 'N') || (byte == 'n')) {
    current_step++;
    if (current_step >= (sizeof(recipe_steps) / sizeof(recipe_steps[0]))) {
      current_step = 0U;
    }
    Show_RenderStep();
    HAL_GPIO_TogglePin(COMM_LED_GPIO_PORT, COMM_LED_PIN);
  }
}

static void Show_SendStatus(void)
{
  Show_UART_WriteString(USART1, "WiFi: CookMirror step ");
  Show_UART_WriteChar(USART1, (char)('1' + current_step));
  Show_UART_WriteString(USART1, "/4\r\n");

  Show_UART_WriteString(USART2, "4G: telemetry OK, send N for next\r\n");
  HAL_GPIO_TogglePin(COMM_LED_GPIO_PORT, COMM_LED_PIN);
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
  Show_GPIO_Init();
  Show_ADC_Init();
  Show_UART_Init();
  LCD_Init();
  Show_RenderStep();
  Show_SendStatus();
  last_status_tick = HAL_GetTick();
  last_contrast_tick = HAL_GetTick();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    uint8_t byte;

    if (Show_UART_ReadByte(USART1, &byte) != 0U) {
      Show_HandleCommand(byte);
    }

    if (Show_UART_ReadByte(USART2, &byte) != 0U) {
      Show_HandleCommand(byte);
    }

    if ((HAL_GetTick() - last_status_tick) >= STATUS_PERIOD_MS) {
      Show_SendStatus();
      last_status_tick = HAL_GetTick();
    }

    if ((HAL_GetTick() - last_contrast_tick) >= CONTRAST_UPDATE_MS) {
      Show_UpdateContrastLine();
      last_contrast_tick = HAL_GetTick();
    }

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
