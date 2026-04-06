/*
 * Copyright (c) 2024 Bosch Sensortec GmbH. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _BMI08X_H
#define _BMI08X_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bmi08.h"

#define BMI085_ACCEL_RANGE_2G  UINT8_C(0x00)
#define BMI085_ACCEL_RANGE_4G  UINT8_C(0x01)
#define BMI085_ACCEL_RANGE_8G  UINT8_C(0x02)
#define BMI085_ACCEL_RANGE_16G UINT8_C(0x03)

#define BMI088_ACCEL_RANGE_3G  UINT8_C(0x00)
#define BMI088_ACCEL_RANGE_6G  UINT8_C(0x01)
#define BMI088_ACCEL_RANGE_12G UINT8_C(0x02)
#define BMI088_ACCEL_RANGE_24G UINT8_C(0x03)

#define BMI085_ACCEL_CHIP_ID UINT8_C(0x1F)
#define BMI088_ACCEL_CHIP_ID UINT8_C(0x1E)

int8_t bmi08xa_init(struct bmi08_dev *dev);
int8_t bmi08xa_set_meas_conf(struct bmi08_dev *dev);
int8_t bmi08xa_perform_selftest(struct bmi08_dev *dev);

#ifdef __cplusplus
}
#endif

#endif /* _BMI08X_H */