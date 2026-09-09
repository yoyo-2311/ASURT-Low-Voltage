/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Milestone 3 - SPI Master
  ******************************************************************************
  */
/* USER CODE END Header */
#include "main.h"
#include <string.h>
#include <stdio.h>

SPI_HandleTypeDef hspi1;
UART_HandleTypeDef huart1;

uint8_t rx_case;
uint8_t spi_tx_buffer[3];
uint8_t spi_rx_buffer[3];

uint32_t received_voltage;
uint32_t received_speed;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);

void Process_Case(uint8_t case_number);
void Send_String(char *str);

void Send_String(char *str)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
}

void Process_Case(uint8_t case_number)
{
    uint16_t case_id;

    if (case_number >= 1 && case_number <= 5)
    {
        case_id = 0x1000 + case_number;

        spi_tx_buffer[0] = (case_id >> 8) & 0xFF;
        spi_tx_buffer[1] = case_id & 0xFF;
        spi_tx_buffer[2] = 0x00;

        /* --- First transaction: sends the request, but the slave's
               reply here is still stale (from before it saw this ID) --- */
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
        HAL_SPI_TransmitReceive(&hspi1, spi_tx_buffer, spi_rx_buffer, 3, HAL_MAX_DELAY);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);

        HAL_Delay(10);

        /* --- Second transaction: by now the slave has decoded the ID
               and pre-loaded the correct response --- */
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
        HAL_SPI_TransmitReceive(&hspi1, spi_tx_buffer, spi_rx_buffer, 3, HAL_MAX_DELAY);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);

        uint32_t message =
            ((uint32_t)spi_rx_buffer[0] << 16) |
            ((uint32_t)spi_rx_buffer[1] << 8) |
            ((uint32_t)spi_rx_buffer[2]);

        received_voltage = (message >> 12) & 0xFFF;
        received_speed = message & 0xFFF;

        char buffer[100];
        sprintf(buffer,
                "\r\nCase ID: 0x%04X\r\nVoltage: %lu.%lu V\r\nWheel Speed: %lu km/h\r\n",
                case_id,
                received_voltage / 10,
                received_voltage % 10,
                received_speed);

        Send_String(buffer);
    }
    else
    {
        Send_String("\r\nInvalid Case! Enter 1 - 5.\r\n");
    }
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_SPI1_Init();
    MX_USART1_UART_Init();

    /* USER CODE BEGIN WHILE */
    Send_String("\r\n==============================\r\n");
    Send_String("   Milestone 3 SPI Master\r\n");
    Send_String("==============================\r\n");
    Send_String("Enter Case ID (1-5):\r\n");

    while (1)
    {
        uint8_t input;

        if (HAL_UART_Receive(&huart1, &input, 1, HAL_MAX_DELAY) == HAL_OK)
        {
            if (input >= '1' && input <= '5')
            {
                rx_case = input - '0';

                HAL_UART_Transmit(&huart1, &input, 1, HAL_MAX_DELAY);
                Send_String("\r\n");

                Process_Case(rx_case);

                Send_String("\r\nEnter Case ID (1-5):\r\n");
            }
            else if (input == '\r' || input == '\n')
            {
                /* Ignore Enter key */
            }
            else
            {
                Send_String("\r\nInvalid input. Enter 1-5.\r\n");
            }
        }
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) Error_Handler();

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) Error_Handler();
}

static void MX_SPI1_Init(void)
{
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 10;
    if (HAL_SPI_Init(&hspi1) != HAL_OK) Error_Handler();
}

static void MX_USART1_UART_Init(void)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 9600;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart1) != HAL_OK) Error_Handler();
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* CS idle HIGH (deselected) until a transaction begins */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);

    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void Error_Handler(void)
{
    __disable_irq();
    while (1) {}
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) {}
#endif
