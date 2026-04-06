/*
 * Copyright (c) 2024 Bosch Sensortec GmbH. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BMI08_DEFS_H_
#define BMI08_DEFS_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef BMI08_INTF_RET_TYPE
#define BMI08_INTF_RET_TYPE int8_t
#endif

#ifndef BMI08_INTF_RET_SUCCESS
#define BMI08_INTF_RET_SUCCESS INT8_C(0)
#endif

#define BMI08_OK                      INT8_C(0)
#define BMI08_E_NULL_PTR              INT8_C(-1)
#define BMI08_E_COM_FAIL              INT8_C(-2)
#define BMI08_E_DEV_NOT_FOUND         INT8_C(-3)
#define BMI08_E_OUT_OF_RANGE          INT8_C(-4)
#define BMI08_E_INVALID_INPUT         INT8_C(-5)
#define BMI08_E_CONFIG_STREAM_ERROR   INT8_C(-6)
#define BMI08_E_RD_WR_LENGTH_INVALID  INT8_C(-7)
#define BMI08_E_INVALID_CONFIG        INT8_C(-8)
#define BMI08_E_FEATURE_NOT_SUPPORTED INT8_C(-9)
#define BMI08_E_SELF_TEST_FAIL        INT8_C(-10)

#define BMI08_SPI_RD_MASK UINT8_C(0x80)
#define BMI08_SPI_WR_MASK UINT8_C(0x7F)

#define BMI08_REG_ACCEL_CHIP_ID UINT8_C(0x00)
#define BMI08_REG_GYRO_CHIP_ID  UINT8_C(0x00)
#define BMI08_GYRO_CHIP_ID      UINT8_C(0x0F)

#define BMI08_ACCEL_ODR_100_HZ UINT8_C(0x08)
#define BMI08_ACCEL_BW_NORMAL  UINT8_C(0x0A)
#define BMI08_ACCEL_PM_ACTIVE  UINT8_C(0x00)
#define BMI08_ACCEL_POWER_ENABLE UINT8_C(0x04)

#define BMI08_GYRO_RANGE_2000_DPS     UINT8_C(0x00)
#define BMI08_GYRO_BW_47_ODR_400_HZ   UINT8_C(0x03)
#define BMI08_GYRO_PM_NORMAL          UINT8_C(0x00)

#define BMI08_SOFT_RESET_CMD UINT8_C(0xB6)

#define BMI08_MS_TO_US(X) UINT32_C((X) * 1000U)

typedef BMI08_INTF_RET_TYPE (*bmi08_read_fptr_t)(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr);
typedef BMI08_INTF_RET_TYPE (*bmi08_write_fptr_t)(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr);
typedef void (*bmi08_delay_us_fptr_t)(uint32_t period, void *intf_ptr);

enum bmi08_intf {
    BMI08_I2C_INTF,
    BMI08_SPI_INTF
};

enum bmi08_variant {
    BMI085_VARIANT = 0,
    BMI088_VARIANT = 1
};

struct bmi08_sensor_data {
    int16_t x;
    int16_t y;
    int16_t z;
};

struct bmi08_cfg {
    uint8_t power;
    uint8_t range;
    uint8_t bw;
    uint8_t odr;
};

struct bmi08_axes_remap {
    uint8_t x_axis;
    uint8_t y_axis;
    uint8_t z_axis;
    uint8_t x_axis_sign;
    uint8_t y_axis_sign;
    uint8_t z_axis_sign;
};

struct bmi08_dev {
    uint8_t accel_chip_id;
    uint8_t gyro_chip_id;
    void *intf_ptr_accel;
    void *intf_ptr_gyro;
    enum bmi08_intf intf;
    enum bmi08_variant variant;
    uint8_t dummy_byte;
    struct bmi08_cfg accel_cfg;
    struct bmi08_cfg gyro_cfg;
    struct bmi08_axes_remap remap;
    const uint8_t *config_file_ptr;
    uint16_t read_write_len;
    bmi08_read_fptr_t read;
    bmi08_write_fptr_t write;
    bmi08_delay_us_fptr_t delay_us;
    BMI08_INTF_RET_TYPE intf_rslt;
};

#endif /* BMI08_DEFS_H_ */