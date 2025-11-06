/*
 * imu_board.c
 *
 *  Created on: Nov 5, 2025
 *      Author: Gabriele Lisci & Johnathan Persaud
 */


#include "inv_imu_transport.h"
#include "main.h"
#include "stm32f4xx_hal.h"

extern SPI_HandleTypeDef hspi1;

int stm32_write_reg(uint8_t reg, const uint8_t *data, uint32_t len)
{
    HAL_GPIO_WritePin(IMU1_CS_GPIO_Port, IMU1_CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, &reg, 1, HAL_MAX_DELAY);
    HAL_SPI_Transmit(&hspi1, (uint8_t *)data, len, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(IMU1_CS_GPIO_Port, IMU1_CS_Pin, GPIO_PIN_SET);
    return 0;
}

int stm32_read_reg(uint8_t reg, uint8_t *data, uint32_t len)
{
    reg |= 0x80; // Read bit for SPI
    HAL_GPIO_WritePin(IMU1_CS_GPIO_Port, IMU1_CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, &reg, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi1, data, len, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(IMU1_CS_GPIO_Port, IMU1_CS_Pin, GPIO_PIN_SET);
    return 0;
}

void stm32_sleep_us(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (SystemCoreClock / 1000000);
    while ((DWT->CYCCNT - start) < ticks);
}

inv_imu_transport_t imu_transport = {
    .read_reg  = stm32_read_reg,
    .write_reg = stm32_write_reg,
    .sleep_us  = stm32_sleep_us,
};


