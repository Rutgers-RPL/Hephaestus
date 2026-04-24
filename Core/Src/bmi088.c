/*
 * bmi088.c
 *
 *  Created on: Oct 26, 2025
 *      Author: Dhruv Shah
 */

#include "bmi088.h"
#include "bmi08_defs.h"
#include "bmi08.h"
#include "stm32f4xx_hal.h"
#include "bmi08x.h"

#include <stdint.h>
#include <math.h>

BMI08_INTF_RET_TYPE bmi088_read_spi(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr) {
	struct bmi088_sensor_intf* sensor_intf = (struct bmi088_sensor_intf*) intf_ptr;

	HAL_GPIO_WritePin(sensor_intf->gpio_port, sensor_intf->gpio_pin, GPIO_PIN_RESET);

	HAL_StatusTypeDef ret = HAL_OK;

	ret = HAL_SPI_Transmit(sensor_intf->spi_handle, &reg_addr, 1, HAL_MAX_DELAY);

	if (ret != HAL_OK) {
	    HAL_GPIO_WritePin(sensor_intf->gpio_port, sensor_intf->gpio_pin, GPIO_PIN_SET);
		return ret;
	}

	ret = HAL_SPI_Receive(sensor_intf->spi_handle, reg_data, len, HAL_MAX_DELAY);

	if (ret != HAL_OK) {
	    HAL_GPIO_WritePin(sensor_intf->gpio_port, sensor_intf->gpio_pin, GPIO_PIN_SET);
		return ret;
	}

    HAL_GPIO_WritePin(sensor_intf->gpio_port, sensor_intf->gpio_pin, GPIO_PIN_SET);

	return 0;
}

BMI08_INTF_RET_TYPE bmi088_write_spi(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr) {
	struct bmi088_sensor_intf* sensor_intf = (struct bmi088_sensor_intf*) intf_ptr;

	HAL_GPIO_WritePin(sensor_intf->gpio_port, sensor_intf->gpio_pin, GPIO_PIN_RESET);

	HAL_StatusTypeDef ret = HAL_OK;

	ret = HAL_SPI_Transmit(sensor_intf->spi_handle, &reg_addr, 1, HAL_MAX_DELAY);

	if (ret != HAL_OK) {
		HAL_GPIO_WritePin(sensor_intf->gpio_port, sensor_intf->gpio_pin, GPIO_PIN_SET);
		return ret;
	}

	ret = HAL_SPI_Transmit(sensor_intf->spi_handle, reg_data, len, HAL_MAX_DELAY);

	if (ret != HAL_OK) {
		HAL_GPIO_WritePin(sensor_intf->gpio_port, sensor_intf->gpio_pin, GPIO_PIN_SET);
		return ret;
	}

    HAL_GPIO_WritePin(sensor_intf->gpio_port, sensor_intf->gpio_pin, GPIO_PIN_SET);

    return 0;
}

void bmi088_delay_us(uint32_t period, void *intf_ptr) {
	HAL_Delay(ceil((double)(period)/(1000.0)));
};

int8_t bmi088_init(struct BMI088* bmi, SPI_HandleTypeDef* spi_handle, 
                   uint16_t accel_cs_pin, GPIO_TypeDef* accel_cs_port,
                   uint16_t gyro_cs_pin, GPIO_TypeDef* gyro_cs_port) {
	HAL_GPIO_WritePin(accel_cs_port, accel_cs_pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(gyro_cs_port, gyro_cs_pin, GPIO_PIN_SET);

	struct bmi08_accel_int_channel_cfg accel_new_data_int_cfg;
	struct bmi08_gyro_int_channel_cfg gyro_new_data_int_cfg;

	bmi->accel_intf.gpio_pin = accel_cs_pin;
	bmi->accel_intf.gpio_port = accel_cs_port;
	bmi->accel_intf.spi_handle = spi_handle;

	bmi->gyro_intf.gpio_pin = gyro_cs_pin;
	bmi->gyro_intf.gpio_port = gyro_cs_port;
	bmi->gyro_intf.spi_handle = spi_handle;

	bmi->dev.intf_ptr_accel = &bmi->accel_intf;
	bmi->dev.intf_ptr_gyro = &bmi->gyro_intf;
	bmi->dev.intf = BMI08_SPI_INTF;
	bmi->dev.variant = BMI088_VARIANT;
	bmi->dev.read_write_len = 8;
	bmi->dev.read = bmi088_read_spi;
	bmi->dev.write = bmi088_write_spi;
	bmi->dev.delay_us = bmi088_delay_us;

	bmi->dev.accel_cfg.power = BMI08_ACCEL_PM_ACTIVE;
	bmi->dev.accel_cfg.range = BMI088_ACCEL_RANGE_24G;
	bmi->dev.accel_cfg.bw = BMI08_ACCEL_BW_NORMAL;
	bmi->dev.accel_cfg.odr = BMI08_ACCEL_ODR_800_HZ;

	bmi->dev.gyro_cfg.power = BMI08_GYRO_PM_NORMAL;
	bmi->dev.gyro_cfg.range = BMI08_GYRO_RANGE_2000_DPS;
	bmi->dev.gyro_cfg.bw = BMI08_GYRO_BW_116_ODR_1000_HZ;
	bmi->dev.gyro_cfg.odr = BMI08_GYRO_BW_116_ODR_1000_HZ;

	accel_new_data_int_cfg.int_channel = BMI08_INT_CHANNEL_1;
	accel_new_data_int_cfg.int_type = BMI08_ACCEL_INT_DATA_RDY;
	accel_new_data_int_cfg.int_pin_cfg.output_mode = BMI08_INT_MODE_PUSH_PULL;
	accel_new_data_int_cfg.int_pin_cfg.lvl = BMI08_INT_ACTIVE_HIGH;
	accel_new_data_int_cfg.int_pin_cfg.enable_int_pin = BMI08_ENABLE;

	gyro_new_data_int_cfg.int_channel = BMI08_INT_CHANNEL_3;
	gyro_new_data_int_cfg.int_type = BMI08_GYRO_INT_DATA_RDY;
	gyro_new_data_int_cfg.int_pin_cfg.output_mode = BMI08_INT_MODE_PUSH_PULL;
	gyro_new_data_int_cfg.int_pin_cfg.lvl = BMI08_INT_ACTIVE_HIGH;
	gyro_new_data_int_cfg.int_pin_cfg.enable_int_pin = BMI08_ENABLE;

	int8_t ret = BMI08_OK;

	ret = bmi08a_soft_reset(&bmi->dev);

	if (ret != BMI08_OK) {
		return ret;
	}

	ret = bmi08xa_init(&bmi->dev);

	if (ret != BMI08_OK) {
		return ret;
	}

	ret = bmi08a_load_config_file(&bmi->dev);

	if (ret != BMI08_OK) {
		return ret;
	}

	ret = bmi08a_set_power_mode(&bmi->dev);

	if (ret != BMI08_OK) {
		return ret;
	}

	ret = bmi08xa_set_meas_conf(&bmi->dev);

	if (ret != BMI08_OK) {
		return ret;
	}

	ret = bmi08a_set_int_config(&accel_new_data_int_cfg, &bmi->dev);

	if (ret != BMI08_OK) {
		return ret;
	}

	ret = bmi08g_soft_reset(&bmi->dev);

	if (ret != BMI08_OK) {
		return ret;
	}

	ret = bmi08g_init(&bmi->dev);

	if (ret != BMI08_OK) {
		return ret;
	}

	ret = bmi08g_set_power_mode(&bmi->dev);

	if (ret != BMI08_OK) {
		return ret;
	}

	ret = bmi08g_set_meas_conf(&bmi->dev);

	if (ret != BMI08_OK) {
		return ret;
	}

	ret = bmi08g_set_int_config(&gyro_new_data_int_cfg, &bmi->dev);

	if (ret != BMI08_OK) {
		return ret;
	}

	return 0;
}

int8_t bmi088_update_accel_data(struct BMI088* bmi, struct bmi08_sensor_data* accel_data) {
	return bmi08a_get_data(accel_data, &bmi->dev);
};

int8_t bmi088_update_gyro_data(struct BMI088* bmi, struct bmi08_sensor_data* gyro_data) {
	return bmi08g_get_data(gyro_data, &bmi->dev);
};

float bmi088_convert_accel_axis_data(struct BMI088* bmi, int16_t axis_data) {
	return ((float)axis_data / 32768.0f * 1000 * pow(2, bmi->dev.accel_cfg.range + 1) * 1.5) * 0.00981;
}

float bmi088_convert_gyro_axis_data(struct BMI088* bmi, int16_t axis_data) {
	switch (bmi->dev.gyro_cfg.range) {
		case BMI08_GYRO_RANGE_2000_DPS:
			return CONVERT_GYRO_RAW_RANGE(axis_data, 2000);
		case BMI08_GYRO_RANGE_1000_DPS:
			return CONVERT_GYRO_RAW_RANGE(axis_data, 1000);
		case BMI08_GYRO_RANGE_500_DPS:
			return CONVERT_GYRO_RAW_RANGE(axis_data, 500);
		case BMI08_GYRO_RANGE_250_DPS:
			return CONVERT_GYRO_RAW_RANGE(axis_data, 250);
		case BMI08_GYRO_RANGE_125_DPS:
			return CONVERT_GYRO_RAW_RANGE(axis_data, 125);
		default:
			return 0.0f;
	}
}
