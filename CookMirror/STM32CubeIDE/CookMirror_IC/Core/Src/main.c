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
#define I2C_SCL_PIN             (1U << 6)   /* PB6 */
#define I2C_SDA_PIN             (1U << 7)   /* PB7 */
#define DHT22_DATA_PIN          (1U << 8)   /* PB8 */
#define STATUS_LED_PIN          (1U << 13)  /* PC13, optional */

#define LM75_ADDR_7BIT          0x48U
#define LM75_TEMP_REG           0x00U

#define UART_BAUDRATE           9600U
#define UART_PCLK_HZ            8000000U
#define SENSOR_PERIOD_MS        3000U
#define I2C_DELAY_US            5U
#define DHT_START_LOW_MS        20U
#define DHT_TIMEOUT_US          1000U
#define DHT_SAMPLE_US           30U
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
static void IC_GPIO_Init(void);
static void IC_TIM2_Init(void);
static void IC_DelayUs(uint32_t us);
static uint8_t IC_WaitPinLevel(GPIO_TypeDef *port, uint32_t pin, uint8_t level, uint32_t timeout_us);

static void IC_UART_Init(void);
static void IC_UART_WriteChar(char ch);
static void IC_UART_WriteString(const char *text);
static void IC_UART_WriteUnsigned(uint32_t value);
static void IC_UART_WriteSignedTenths(int16_t value);

static void IC_I2C_Start(void);
static void IC_I2C_Stop(void);
static uint8_t IC_I2C_WriteByte(uint8_t data);
static uint8_t IC_I2C_ReadByte(uint8_t ack);
static uint8_t IC_LM75_ReadTenths(int16_t *temp_tenths);
static int16_t IC_LM75_ToTenths(uint8_t msb, uint8_t lsb);

static void IC_DHT22_PinOutput(void);
static void IC_DHT22_PinInput(void);
static uint8_t IC_DHT22_Read(int16_t *temp_tenths, uint16_t *humidity_tenths);
static void IC_PrintSensorLine(uint8_t lm75_ok, int16_t lm75_temp,
                               uint8_t dht_ok, int16_t dht_temp,
                               uint16_t dht_humidity);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static uint8_t dht_last_error = 0U;
static uint8_t dht_error_bit = 0U;

static void IC_GPIO_Init(void)
{
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN |
                  RCC_APB2ENR_IOPCEN | RCC_APB2ENR_AFIOEN;

  /* PB6/PB7: software I2C open-drain outputs, released high by default. */
  GPIOB->CRL &= ~((0xFU << 24) | (0xFU << 28));
  GPIOB->CRL |=  ((0x6U << 24) | (0x6U << 28));
  GPIOB->BSRR = I2C_SCL_PIN | I2C_SDA_PIN;

  IC_DHT22_PinInput();

  /* PC13 is optional; connect an LED if you want a heartbeat. */
  GPIOC->CRH &= ~(0xFU << 20);
  GPIOC->CRH |=  (0x2U << 20);
  GPIOC->BSRR = STATUS_LED_PIN;
}

static void IC_TIM2_Init(void)
{
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
  TIM2->PSC = (uint16_t)((SystemCoreClock / 1000000U) - 1U);
  TIM2->ARR = 0xFFFFU;
  TIM2->EGR = TIM_EGR_UG;
  TIM2->CR1 = TIM_CR1_CEN;
}

static void IC_DelayUs(uint32_t us)
{
  uint16_t start;

  while (us > 0U) {
    uint16_t chunk = (us > 60000U) ? 60000U : (uint16_t)us;
    start = (uint16_t)TIM2->CNT;
    while ((uint16_t)(TIM2->CNT - start) < chunk) {
    }
    us -= chunk;
  }
}

static uint8_t IC_WaitPinLevel(GPIO_TypeDef *port, uint32_t pin, uint8_t level, uint32_t timeout_us)
{
  uint16_t start = (uint16_t)TIM2->CNT;

  while (((port->IDR & pin) != 0U) != (level != 0U)) {
    if ((uint16_t)(TIM2->CNT - start) > timeout_us) {
      return 0U;
    }
  }

  return 1U;
}

static void IC_UART_Init(void)
{
  uint32_t brr = (UART_PCLK_HZ + (UART_BAUDRATE / 2U)) / UART_BAUDRATE;

  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN | RCC_APB2ENR_AFIOEN;

  /* PA9: USART1_TX alternate-function push-pull. PA10: RX input floating. */
  GPIOA->CRH &= ~((0xFU << 4) | (0xFU << 8));
  GPIOA->CRH |=  ((0xAU << 4) | (0x4U << 8));

  USART1->BRR = brr;
  USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

static void IC_UART_WriteChar(char ch)
{
  while ((USART1->SR & USART_SR_TXE) == 0U) {
  }
  USART1->DR = (uint16_t)ch;
}

static void IC_UART_WriteString(const char *text)
{
  while (*text != '\0') {
    IC_UART_WriteChar(*text);
    text++;
  }
}

static void IC_UART_WriteUnsigned(uint32_t value)
{
  char buffer[10];
  uint8_t index = 0U;

  if (value == 0U) {
    IC_UART_WriteChar('0');
    return;
  }

  while (value > 0U) {
    buffer[index++] = (char)('0' + (value % 10U));
    value /= 10U;
  }

  while (index > 0U) {
    IC_UART_WriteChar(buffer[--index]);
  }
}

static void IC_UART_WriteSignedTenths(int16_t value)
{
  uint16_t magnitude;

  if (value < 0) {
    IC_UART_WriteChar('-');
    magnitude = (uint16_t)(-value);
  } else {
    magnitude = (uint16_t)value;
  }

  IC_UART_WriteUnsigned(magnitude / 10U);
  IC_UART_WriteChar('.');
  IC_UART_WriteChar((char)('0' + (magnitude % 10U)));
}

static void IC_I2C_Start(void)
{
  GPIOB->BSRR = I2C_SDA_PIN | I2C_SCL_PIN;
  IC_DelayUs(I2C_DELAY_US);
  GPIOB->BRR = I2C_SDA_PIN;
  IC_DelayUs(I2C_DELAY_US);
  GPIOB->BRR = I2C_SCL_PIN;
}

static void IC_I2C_Stop(void)
{
  GPIOB->BRR = I2C_SDA_PIN;
  IC_DelayUs(I2C_DELAY_US);
  GPIOB->BSRR = I2C_SCL_PIN;
  IC_DelayUs(I2C_DELAY_US);
  GPIOB->BSRR = I2C_SDA_PIN;
  IC_DelayUs(I2C_DELAY_US);
}

static uint8_t IC_I2C_WriteByte(uint8_t data)
{
  uint8_t i;
  uint8_t ack;

  for (i = 0U; i < 8U; i++) {
    if ((data & 0x80U) != 0U) {
      GPIOB->BSRR = I2C_SDA_PIN;
    } else {
      GPIOB->BRR = I2C_SDA_PIN;
    }
    IC_DelayUs(I2C_DELAY_US);
    GPIOB->BSRR = I2C_SCL_PIN;
    IC_DelayUs(I2C_DELAY_US);
    GPIOB->BRR = I2C_SCL_PIN;
    data <<= 1;
  }

  GPIOB->BSRR = I2C_SDA_PIN;
  IC_DelayUs(I2C_DELAY_US);
  GPIOB->BSRR = I2C_SCL_PIN;
  IC_DelayUs(I2C_DELAY_US);
  ack = ((GPIOB->IDR & I2C_SDA_PIN) == 0U) ? 1U : 0U;
  GPIOB->BRR = I2C_SCL_PIN;
  return ack;
}

static uint8_t IC_I2C_ReadByte(uint8_t ack)
{
  uint8_t i;
  uint8_t data = 0U;

  GPIOB->BSRR = I2C_SDA_PIN;
  for (i = 0U; i < 8U; i++) {
    data <<= 1;
    GPIOB->BSRR = I2C_SCL_PIN;
    IC_DelayUs(I2C_DELAY_US);
    if ((GPIOB->IDR & I2C_SDA_PIN) != 0U) {
      data |= 1U;
    }
    GPIOB->BRR = I2C_SCL_PIN;
    IC_DelayUs(I2C_DELAY_US);
  }

  if (ack != 0U) {
    GPIOB->BRR = I2C_SDA_PIN;
  } else {
    GPIOB->BSRR = I2C_SDA_PIN;
  }
  IC_DelayUs(I2C_DELAY_US);
  GPIOB->BSRR = I2C_SCL_PIN;
  IC_DelayUs(I2C_DELAY_US);
  GPIOB->BRR = I2C_SCL_PIN;
  GPIOB->BSRR = I2C_SDA_PIN;

  return data;
}

static int16_t IC_LM75_ToTenths(uint8_t msb, uint8_t lsb)
{
  int16_t raw9 = (int16_t)(((uint16_t)msb << 1) | ((uint16_t)lsb >> 7));

  if ((raw9 & 0x0100) != 0) {
    raw9 |= (int16_t)~0x01FF;
  }

  return (int16_t)(raw9 * 5);
}

static uint8_t IC_LM75_ReadTenths(int16_t *temp_tenths)
{
  uint8_t msb;
  uint8_t lsb;

  IC_I2C_Start();
  if (IC_I2C_WriteByte((uint8_t)(LM75_ADDR_7BIT << 1)) == 0U) {
    IC_I2C_Stop();
    return 0U;
  }
  if (IC_I2C_WriteByte(LM75_TEMP_REG) == 0U) {
    IC_I2C_Stop();
    return 0U;
  }

  IC_I2C_Start();
  if (IC_I2C_WriteByte((uint8_t)((LM75_ADDR_7BIT << 1) | 1U)) == 0U) {
    IC_I2C_Stop();
    return 0U;
  }

  msb = IC_I2C_ReadByte(1U);
  lsb = IC_I2C_ReadByte(0U);
  IC_I2C_Stop();

  *temp_tenths = IC_LM75_ToTenths(msb, lsb);
  return 1U;
}

static void IC_DHT22_PinOutput(void)
{
  GPIOB->CRH &= ~(0xFU << 0);
  GPIOB->CRH |=  (0x6U << 0);
}

static void IC_DHT22_PinInput(void)
{
  GPIOB->BSRR = DHT22_DATA_PIN;
  GPIOB->CRH &= ~(0xFU << 0);
  GPIOB->CRH |=  (0x8U << 0);
}

static uint8_t IC_DHT22_Read(int16_t *temp_tenths, uint16_t *humidity_tenths)
{
  uint8_t data[5] = {0U, 0U, 0U, 0U, 0U};
  uint8_t bit;
  uint8_t byte_index;
  uint16_t raw_temp;

  dht_last_error = 0U;
  dht_error_bit = 0U;

  IC_DHT22_PinOutput();
  GPIOB->BRR = DHT22_DATA_PIN;
  HAL_Delay(DHT_START_LOW_MS);
  GPIOB->BSRR = DHT22_DATA_PIN;
  IC_DelayUs(30U);
  IC_DHT22_PinInput();

  if (IC_WaitPinLevel(GPIOB, DHT22_DATA_PIN, 0U, DHT_TIMEOUT_US) == 0U) {
    dht_last_error = 1U;
    return 0U;
  }
  if (IC_WaitPinLevel(GPIOB, DHT22_DATA_PIN, 1U, DHT_TIMEOUT_US) == 0U) {
    dht_last_error = 2U;
    return 0U;
  }
  if (IC_WaitPinLevel(GPIOB, DHT22_DATA_PIN, 0U, DHT_TIMEOUT_US) == 0U) {
    dht_last_error = 3U;
    return 0U;
  }

  for (bit = 0U; bit < 40U; bit++) {
    if (IC_WaitPinLevel(GPIOB, DHT22_DATA_PIN, 1U, DHT_TIMEOUT_US) == 0U) {
      dht_last_error = 4U;
      dht_error_bit = (uint8_t)(bit + 1U);
      return 0U;
    }

    byte_index = bit / 8U;
    data[byte_index] <<= 1;
    IC_DelayUs(DHT_SAMPLE_US);
    if ((GPIOB->IDR & DHT22_DATA_PIN) != 0U) {
      data[byte_index] |= 1U;
    }

    if (IC_WaitPinLevel(GPIOB, DHT22_DATA_PIN, 0U, DHT_TIMEOUT_US) == 0U) {
      dht_last_error = 5U;
      dht_error_bit = (uint8_t)(bit + 1U);
      return 0U;
    }
  }

  if ((((uint16_t)data[0] + data[1] + data[2] + data[3]) & 0xFFU) != data[4]) {
    dht_last_error = 6U;
    return 0U;
  }

  *humidity_tenths = (uint16_t)(((uint16_t)data[0] << 8) | data[1]);
  raw_temp = (uint16_t)(((uint16_t)data[2] << 8) | data[3]);
  if ((raw_temp & 0x8000U) != 0U) {
    *temp_tenths = -(int16_t)(raw_temp & 0x7FFFU);
  } else {
    *temp_tenths = (int16_t)raw_temp;
  }

  return 1U;
}

static void IC_PrintSensorLine(uint8_t lm75_ok, int16_t lm75_temp,
                               uint8_t dht_ok, int16_t dht_temp,
                               uint16_t dht_humidity)
{
  IC_UART_WriteString("LM75: ");
  if (lm75_ok != 0U) {
    IC_UART_WriteSignedTenths(lm75_temp);
    IC_UART_WriteString("C");
  } else {
    IC_UART_WriteString("ERR");
  }

  IC_UART_WriteString(" | DHT22: ");
  if (dht_ok != 0U) {
    IC_UART_WriteSignedTenths(dht_temp);
    IC_UART_WriteString("C ");
    IC_UART_WriteUnsigned(dht_humidity / 10U);
    IC_UART_WriteChar('.');
    IC_UART_WriteChar((char)('0' + (dht_humidity % 10U)));
    IC_UART_WriteString("%");
  } else {
    IC_UART_WriteString("ERR");
    IC_UART_WriteUnsigned(dht_last_error);
    if (dht_error_bit != 0U) {
      IC_UART_WriteChar('@');
      IC_UART_WriteUnsigned(dht_error_bit);
    }
  }
  IC_UART_WriteString("\r\n");
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
  IC_GPIO_Init();
  IC_TIM2_Init();
  IC_UART_Init();
  IC_UART_WriteString("CookMirror IC sensor bus\r\n");
  IC_UART_WriteString("LM75: PB6/PB7, DHT22: PB8\r\n");

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    int16_t lm75_temp = 0;
    int16_t dht_temp = 0;
    uint16_t dht_humidity = 0U;
    uint8_t lm75_ok = IC_LM75_ReadTenths(&lm75_temp);
    uint8_t dht_ok = IC_DHT22_Read(&dht_temp, &dht_humidity);

    IC_PrintSensorLine(lm75_ok, lm75_temp, dht_ok, dht_temp, dht_humidity);
    GPIOC->ODR ^= STATUS_LED_PIN;
    HAL_Delay(SENSOR_PERIOD_MS);

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
