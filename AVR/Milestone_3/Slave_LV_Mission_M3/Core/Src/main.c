/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Milestone 3 - SPI Slave
  ******************************************************************************
  */
/* USER CODE END Header */
#include "main.h"

SPI_HandleTypeDef hspi1;

typedef struct
{
    uint16_t id;
    uint16_t voltage;
    uint16_t speed;
} CaseData;

CaseData cases[5] =
{
    {0x1001, 123, 200},
    {0x1002, 142, 150},
    {0x1003, 138, 100},
    {0x1004, 115, 220},
    {0x1005, 109, 280}
};

uint8_t spi_rx_buffer[3];
uint8_t spi_tx_buffer[3];

uint16_t received_id;
uint32_t encoded_message;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);

void Encode_Case(uint16_t id)
{
    uint16_t voltage = 0;
    uint16_t speed = 0;

    for (int i = 0; i < 5; i++)
    {
        if (cases[i].id == id)
        {
            voltage = cases[i].voltage;
            speed = cases[i].speed;
            break;
        }
    }

    encoded_message = ((uint32_t)(voltage & 0xFFF) << 12) | (speed & 0xFFF);

    spi_tx_buffer[0] = (encoded_message >> 16) & 0xFF;
    spi_tx_buffer[1] = (encoded_message >> 8) & 0xFF;
    spi_tx_buffer[2] = encoded_message & 0xFF;
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_SPI1_Init();

    /* USER CODE BEGIN WHILE */
    while (1)
    {
        /* Blocks until the Master starts a transaction.
           Simultaneously transmits whatever spi_tx_buffer
           currently holds (stale on the very first exchange). */
        HAL_SPI_TransmitReceive(&hspi1, spi_tx_buffer, spi_rx_buffer, 3, HAL_MAX_DELAY);

        /* Decode the Case ID the Master just requested */
        received_id = ((uint16_t)spi_rx_buffer[0] << 8) | spi_rx_buffer[1];

        /* Pre-load the correct response so it's ready
           for the Master's NEXT transaction */
        Encode_Case(received_id);
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
    hspi1.Init.Mode = SPI_MODE_SLAVE;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_HARD_INPUT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 10;
    if (HAL_SPI_Init(&hspi1) != HAL_OK) Error_Handler();
}

static void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
}

void Error_Handler(void)
{
    __disable_irq();
    while (1) {}
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) {}
#endif
