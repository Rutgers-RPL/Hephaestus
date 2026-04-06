/*
 * Copyright (c) 2024 Bosch Sensortec GmbH. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _BMI08_H
#define _BMI08_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bmi08_defs.h"

int8_t bmi08a_init(struct bmi08_dev *dev);
int8_t bmi08a_load_config_file(struct bmi08_dev *dev);
int8_t bmi08a_get_set_regs(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, struct bmi08_dev *dev, uint8_t select);
int8_t bmi08a_set_meas_conf(struct bmi08_dev *dev);
int8_t bmi08a_set_power_mode(struct bmi08_dev *dev);
int8_t bmi08a_get_data(struct bmi08_sensor_data *accel, struct bmi08_dev *dev);

int8_t bmi08g_init(struct bmi08_dev *dev);
int8_t bmi08g_get_regs(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, struct bmi08_dev *dev);
int8_t bmi08g_set_regs(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, struct bmi08_dev *dev);
int8_t bmi08g_set_meas_conf(struct bmi08_dev *dev);
int8_t bmi08g_set_power_mode(struct bmi08_dev *dev);
int8_t bmi08g_get_data(struct bmi08_sensor_data *gyro, struct bmi08_dev *dev);

#ifdef __cplusplus
}
#endif

#endif /* _BMI08_H */