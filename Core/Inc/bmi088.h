/*
 * bmi088.h
 *
 *  Created on: Oct 26, 2025
 *      Author: Dhruv Shah
 */

#ifndef INC_BMI088_H_
#define INC_BMI088_H_

#include "bmi08x.h"
#include "stm32f4xx_hal.h"
#include "bmi08_defs.h"
#include "main.h"

#include <stdint.h>
#include <math.h>

#define CONVERT_GYRO_RAW_RANGE(raw, range) ((((float)raw * (float)range) / 32768.0f) * (M_PI / 180.0f))

// IMU1 pins (from main.h)
#define BMI088_IMU1_CS_PIN IMU1_CS_Pin
#define BMI088_IMU1_CS_PORT IMU1_CS_GPIO_Port
#define BMI088_IMU1_INT_PIN IMU1_INT_Pin
#define BMI088_IMU1_INT_PORT IMU1_INT_GPIO_Port

// IMU2 pins (from main.h) - separate accel and gyro CS
#define BMI088_IMU2_GYRO_CS_PIN IMU2_GYRO_CS_Pin
#define BMI088_IMU2_GYRO_CS_PORT IMU2_GYRO_CS_GPIO_Port
#define BMI088_IMU2_ACC_CS_PIN IMU2_ACC_CS_Pin
#define BMI088_IMU2_ACC_CS_PORT IMU2_ACC_CS_GPIO_Port
#define BMI088_IMU2_INT_PIN IMU2_INT_Pin
#define BMI088_IMU2_INT_PORT IMU2_INT_GPIO_Port

struct bmi088_sensor_intf {
	uint16_t gpio_pin;
	GPIO_TypeDef* gpio_port;
	SPI_HandleTypeDef* spi_handle;
};

struct BMI088 {
	struct bmi08_dev dev;
	struct bmi088_sensor_intf accel_intf;
	struct bmi088_sensor_intf gyro_intf;
};

BMI08_INTF_RET_TYPE bmi088_read_spi(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr);
BMI08_INTF_RET_TYPE bmi088_write_spi(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr);
void bmi088_delay_us(uint32_t period, void *intf_ptr);
int8_t bmi088_init(struct BMI088* bmi, SPI_HandleTypeDef* spi_handle,
                   uint16_t accel_cs_pin, GPIO_TypeDef* accel_cs_port,
                   uint16_t gyro_cs_pin, GPIO_TypeDef* gyro_cs_port);
int8_t bmi088_update_accel_data(struct BMI088* bmi, struct bmi08_sensor_data* accel_data);
int8_t bmi088_update_gyro_data(struct BMI088* bmi, struct bmi08_sensor_data* gyro_data);
float bmi088_convert_accel_axis_data(struct BMI088* bmi, int16_t axis_data);
float bmi088_convert_gyro_axis_data(struct BMI088* bmi, int16_t axis_data);

#endif /* INC_BMI088_H_ */
