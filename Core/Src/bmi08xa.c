/**
 * Copyright (c) 2024 Bosch Sensortec GmbH. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "bmi08x.h"

struct bmi08xa_selftest_delta_limit
{
    uint16_t x;
    uint16_t y;
    uint16_t z;
};

/* NOTE: trimmed config blob for initial port scaffolding. */
const uint8_t bmi08x_config_file[] = { 0xc8, 0x2e, 0x00, 0x2e, 0x80, 0x2e, 0x48, 0xb4 };

static int8_t null_ptr_check(const struct bmi08_dev *dev);
static int8_t enable_self_test(struct bmi08_dev *dev);
static int8_t positive_excited_accel(struct bmi08_sensor_data *accel_pos, struct bmi08_dev *dev);
static int8_t negative_excited_accel(struct bmi08_sensor_data *accel_neg, struct bmi08_dev *dev);
static int8_t validate_accel_self_test(const struct bmi08_sensor_data *accel_pos,
                                       const struct bmi08_sensor_data *accel_neg,
                                       uint8_t variant);
static int8_t set_range(struct bmi08_dev *dev);

int8_t bmi08xa_init(struct bmi08_dev *dev)
{
    int8_t rslt = bmi08a_init(dev);

    if (rslt == BMI08_OK)
    {
        if (((dev->variant == BMI085_VARIANT) && (dev->accel_chip_id == BMI085_ACCEL_CHIP_ID)) ||
            ((dev->variant == BMI088_VARIANT) && (dev->accel_chip_id == BMI088_ACCEL_CHIP_ID)))
        {
            dev->config_file_ptr = bmi08x_config_file;
        }
        else
        {
            rslt = BMI08_E_DEV_NOT_FOUND;
        }
    }

    return rslt;
}

int8_t bmi08xa_set_meas_conf(struct bmi08_dev *dev)
{
    int8_t rslt = bmi08a_set_meas_conf(dev);
    if (rslt == BMI08_OK)
    {
        rslt = set_range(dev);
    }

    return rslt;
}

int8_t bmi08xa_configure_data_synchronization(struct bmi08_data_sync_cfg sync_cfg, struct bmi08_dev *dev)
{
    int8_t rslt = null_ptr_check(dev);

    if (rslt == BMI08_OK)
    {
        rslt = bmi08a_configure_data_synchronization(sync_cfg, dev);

        if (rslt == BMI08_OK)
        {
            rslt = set_range(dev);
            dev->delay_us(100000, dev->intf_ptr_accel);
        }
    }

    return rslt;
}

int8_t bmi08xa_perform_selftest(struct bmi08_dev *dev)
{
    int8_t rslt;
    int8_t self_test_rslt = 0;
    struct bmi08_sensor_data accel_pos, accel_neg;

    rslt = null_ptr_check(dev);
    if (rslt != BMI08_OK)
    {
        return rslt;
    }

    rslt = enable_self_test(dev);
    if (rslt == BMI08_OK)
    {
        rslt = positive_excited_accel(&accel_pos, dev);
    }
    if (rslt == BMI08_OK)
    {
        rslt = negative_excited_accel(&accel_neg, dev);
    }
    if (rslt == BMI08_OK)
    {
        rslt = validate_accel_self_test(&accel_pos, &accel_neg, dev->variant);
        self_test_rslt = rslt;

        rslt = bmi08a_soft_reset(dev);
        if (rslt == BMI08_OK)
        {
            rslt = self_test_rslt;
        }
    }

    return rslt;
}

static int8_t null_ptr_check(const struct bmi08_dev *dev)
{
    if ((dev == NULL) || (dev->read == NULL) || (dev->write == NULL) || (dev->delay_us == NULL))
    {
        return BMI08_E_NULL_PTR;
    }

    return BMI08_OK;
}

static int8_t enable_self_test(struct bmi08_dev *dev)
{
    int8_t rslt;

    dev->accel_cfg.odr = BMI08_ACCEL_ODR_1600_HZ;
    dev->accel_cfg.bw = BMI08_ACCEL_BW_NORMAL;

    if (dev->variant == BMI085_VARIANT)
    {
        dev->accel_cfg.range = BMI085_ACCEL_RANGE_16G;
    }
    else if (dev->variant == BMI088_VARIANT)
    {
        dev->accel_cfg.range = BMI088_ACCEL_RANGE_24G;
    }

    dev->accel_cfg.power = BMI08_ACCEL_PM_ACTIVE;

    rslt = bmi08a_set_power_mode(dev);
    if (rslt == BMI08_OK)
    {
        rslt = bmi08xa_set_meas_conf(dev);
        if (rslt == BMI08_OK)
        {
            dev->delay_us(BMI08_MS_TO_US(BMI08_SELF_TEST_DELAY_MS), dev->intf_ptr_accel);
        }
    }

    return rslt;
}

static int8_t positive_excited_accel(struct bmi08_sensor_data *accel_pos, struct bmi08_dev *dev)
{
    int8_t rslt;
    uint8_t reg_data = BMI08_ACCEL_POSITIVE_SELF_TEST;

    rslt = bmi08a_get_set_regs(BMI08_REG_ACCEL_SELF_TEST, &reg_data, BMI08_REG_ACCEL_SELF_TEST_LENGTH, dev, SET_FUNC);
    if (rslt == BMI08_OK)
    {
        dev->delay_us(BMI08_MS_TO_US(BMI08_SELF_TEST_DATA_READ_MS), dev->intf_ptr_accel);
        rslt = bmi08a_get_data(accel_pos, dev);
    }

    return rslt;
}

static int8_t negative_excited_accel(struct bmi08_sensor_data *accel_neg, struct bmi08_dev *dev)
{
    int8_t rslt;
    uint8_t reg_data = BMI08_ACCEL_NEGATIVE_SELF_TEST;

    rslt = bmi08a_get_set_regs(BMI08_REG_ACCEL_SELF_TEST, &reg_data, BMI08_REG_ACCEL_SELF_TEST_LENGTH, dev, SET_FUNC);
    if (rslt == BMI08_OK)
    {
        dev->delay_us(BMI08_MS_TO_US(BMI08_SELF_TEST_DATA_READ_MS), dev->intf_ptr_accel);
        rslt = bmi08a_get_data(accel_neg, dev);

        if (rslt == BMI08_OK)
        {
            reg_data = BMI08_ACCEL_SWITCH_OFF_SELF_TEST;
            rslt = bmi08a_get_set_regs(BMI08_REG_ACCEL_SELF_TEST,
                                       &reg_data,
                                       BMI08_REG_ACCEL_SELF_TEST_LENGTH,
                                       dev,
                                       SET_FUNC);
        }
    }

    return rslt;
}

static int8_t validate_accel_self_test(const struct bmi08_sensor_data *accel_pos,
                                       const struct bmi08_sensor_data *accel_neg,
                                       uint8_t variant)
{
    int32_t lsb_per_g = 0;
    struct bmi08xa_selftest_delta_limit accel_data_diff = { 0 };
    struct bmi08xa_selftest_delta_limit accel_data_diff_mg = { 0 };

    accel_data_diff.x = (uint16_t)BMI08_ABS(accel_pos->x - accel_neg->x);
    accel_data_diff.y = (uint16_t)BMI08_ABS(accel_pos->y - accel_neg->y);
    accel_data_diff.z = (uint16_t)BMI08_ABS(accel_pos->z - accel_neg->z);

    if (variant == BMI085_VARIANT)
    {
        lsb_per_g = INT32_C(2048);
    }
    else if (variant == BMI088_VARIANT)
    {
        lsb_per_g = INT32_C(1365);
    }

    if (lsb_per_g == 0)
    {
        return BMI08_E_SELF_TEST_FAIL;
    }

    accel_data_diff_mg.x = (uint16_t)((accel_data_diff.x / lsb_per_g) * 1000);
    accel_data_diff_mg.y = (uint16_t)((accel_data_diff.y / lsb_per_g) * 1000);
    accel_data_diff_mg.z = (uint16_t)((accel_data_diff.z / lsb_per_g) * 1000);

    return (accel_data_diff_mg.x >= 1000 && accel_data_diff_mg.y >= 1000 && accel_data_diff_mg.z >= 500) ?
               BMI08_OK :
               BMI08_E_SELF_TEST_FAIL;
}

static int8_t set_range(struct bmi08_dev *dev)
{
    int8_t rslt;
    uint8_t data = 0;
    uint8_t range = dev->accel_cfg.range;
    uint8_t is_range_invalid = FALSE;

    if ((dev->variant == BMI085_VARIANT) && (range > BMI085_ACCEL_RANGE_16G))
    {
        is_range_invalid = TRUE;
    }

    if ((dev->variant == BMI088_VARIANT) && (range > BMI088_ACCEL_RANGE_24G))
    {
        is_range_invalid = TRUE;
    }

    if (!is_range_invalid)
    {
        rslt = bmi08a_get_set_regs(BMI08_REG_ACCEL_RANGE, &data, BMI08_REG_ACCEL_RANGE_LENGTH, dev, GET_FUNC);
        if (rslt == BMI08_OK)
        {
            data = BMI08_SET_BITS_POS_0(data, BMI08_ACCEL_RANGE, range);
            rslt = bmi08a_get_set_regs(BMI08_REG_ACCEL_RANGE, &data, BMI08_REG_ACCEL_RANGE_LENGTH, dev, SET_FUNC);
        }
    }
    else
    {
        rslt = BMI08_E_INVALID_CONFIG;
    }

    return rslt;
}