/**
* Copyright (c) 2024 Bosch Sensortec GmbH. All rights reserved.
*
* BSD-3-Clause
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* 3. Neither the name of the copyright holder nor the names of its
*    contributors may be used to endorse or promote products derived from
*    this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
* IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
* @file       bmi08g.c
* @date       2024-07-29
* @version    v1.9.0
*
*/

#include "bmi08.h"

static int8_t null_ptr_check(const struct bmi08_dev *dev);
static int8_t get_regs(uint8_t reg_addr, uint8_t *data, uint32_t len, struct bmi08_dev *dev);
static int8_t set_regs(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, struct bmi08_dev *dev);
static int8_t set_gyro_data_ready_int(const struct bmi08_gyro_int_channel_cfg *int_config, struct bmi08_dev *dev);
static int8_t set_fifo_int(const struct bmi08_gyro_int_channel_cfg *int_config, struct bmi08_dev *dev);
static int8_t set_int_pin_config(const struct bmi08_gyro_int_channel_cfg *int_config, struct bmi08_dev *dev);
static int8_t set_gyro_selftest(uint8_t selftest, struct bmi08_dev *dev);
static void get_fifo_data_length(const struct bmi08_gyr_fifo_config *fifo, int8_t frame_size, uint16_t *fifo_data_byte);
static void parse_fifo_gyro_len(uint16_t *len,
                                const uint16_t *gyr_count,
                                const struct bmi08_gyr_fifo_config *fifo_conf,
                                const struct bmi08_fifo_frame *fifo);
static void unpack_gyro_data(struct bmi08_sensor_data *gyro,
                             uint16_t *data_index,
                             const struct bmi08_gyr_fifo_config *fifo_conf,
                             const struct bmi08_fifo_frame *fifo);

int8_t bmi08g_init(struct bmi08_dev *dev)
{
    int8_t rslt;
    uint8_t chip_id = 0;

    rslt = null_ptr_check(dev);

    if (rslt == BMI08_OK)
    {
        dev->gyro_chip_id = 0;
        rslt = get_regs(BMI08_REG_GYRO_CHIP_ID, &chip_id, BMI08_REG_GYRO_CHIP_ID_LENGTH, dev);

        if (rslt == BMI08_OK)
        {
            if (chip_id == BMI08_GYRO_CHIP_ID)
            {
                dev->gyro_chip_id = chip_id;
            }
            else
            {
                rslt = BMI08_E_DEV_NOT_FOUND;
            }
        }
    }

    return rslt;
}

int8_t bmi08g_get_regs(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, struct bmi08_dev *dev)
{
    int8_t rslt;

    rslt = null_ptr_check(dev);

    if ((rslt == BMI08_OK) && (reg_data != NULL))
    {
        if (len > 0)
        {
            rslt = get_regs(reg_addr, reg_data, len, dev);
        }
        else
        {
            rslt = BMI08_E_RD_WR_LENGTH_INVALID;
        }
    }
    else
    {
        rslt = BMI08_E_NULL_PTR;
    }

    return rslt;
}

int8_t bmi08g_set_regs(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, struct bmi08_dev *dev)
{
    int8_t rslt;

    rslt = null_ptr_check(dev);

    if ((rslt == BMI08_OK) && (reg_data != NULL))
    {
        if (len > 0)
        {
            rslt = set_regs(reg_addr, reg_data, len, dev);

            if (dev->gyro_cfg.power == BMI08_GYRO_PM_SUSPEND || dev->gyro_cfg.power == BMI08_GYRO_PM_DEEP_SUSPEND)
            {
                dev->delay_us(450, dev->intf_ptr_gyro);
            }
            else if (dev->gyro_cfg.power == BMI08_GYRO_PM_NORMAL)
            {
                dev->delay_us(2, dev->intf_ptr_gyro);
            }
            else
            {
                rslt = BMI08_E_INVALID_INPUT;
            }
        }
        else
        {
            rslt = BMI08_E_RD_WR_LENGTH_INVALID;
        }
    }
    else
    {
        rslt = BMI08_E_NULL_PTR;
    }

    return rslt;
}

int8_t bmi08g_soft_reset(struct bmi08_dev *dev)
{
    int8_t rslt;
    uint8_t data;

    rslt = null_ptr_check(dev);

    if (rslt == BMI08_OK)
    {
        data = BMI08_SOFT_RESET_CMD;
        rslt = bmi08g_set_regs(BMI08_REG_GYRO_SOFTRESET, &data, BMI08_REG_GYRO_SOFTRESET_LENGTH, dev);

        if (rslt == BMI08_OK)
        {
            dev->delay_us(BMI08_MS_TO_US(BMI08_GYRO_SOFTRESET_DELAY), dev->intf_ptr_gyro);
        }
    }

    return rslt;
}

int8_t bmi08g_get_meas_conf(struct bmi08_dev *dev)
{
    int8_t rslt;
    uint8_t data[2];

    rslt = null_ptr_check(dev);

    if (rslt == BMI08_OK)
    {
        rslt = bmi08g_get_regs(BMI08_REG_GYRO_RANGE, data, (BMI08_REG_GYRO_RANGE_LENGTH - 1), dev);

        if (rslt == BMI08_OK)
        {
            dev->gyro_cfg.range = data[0];
            dev->gyro_cfg.odr = (data[1] & BMI08_GYRO_BW_MASK);
            dev->gyro_cfg.bw = dev->gyro_cfg.odr;
        }
    }

    return rslt;
}

int8_t bmi08g_set_meas_conf(struct bmi08_dev *dev)
{
    int8_t rslt;
    uint8_t data;
    uint8_t odr, range;
    uint8_t is_range_invalid = FALSE, is_odr_invalid = FALSE;

    rslt = null_ptr_check(dev);

    if (rslt == BMI08_OK)
    {
        odr = dev->gyro_cfg.odr;
        range = dev->gyro_cfg.range;

        if (odr > BMI08_GYRO_BW_32_ODR_100_HZ)
        {
            is_odr_invalid = TRUE;
        }

        if (range > BMI08_GYRO_RANGE_125_DPS)
        {
            is_range_invalid = TRUE;
        }

        if ((!is_odr_invalid) && (!is_range_invalid))
        {
            rslt = bmi08g_get_regs(BMI08_REG_GYRO_BANDWIDTH, &data, BMI08_REG_GYRO_BANDWIDTH_LENGTH, dev);

            if (rslt == BMI08_OK)
            {
                data = BMI08_SET_BITS_POS_0(data, BMI08_GYRO_BW, odr);
                rslt = bmi08g_set_regs(BMI08_REG_GYRO_BANDWIDTH, &data, BMI08_REG_GYRO_BANDWIDTH_LENGTH, dev);

                if (rslt == BMI08_OK)
                {
                    rslt = bmi08g_get_regs(BMI08_REG_GYRO_RANGE, &data, (BMI08_REG_GYRO_RANGE_LENGTH - 2), dev);
                }

                if (rslt == BMI08_OK)
                {
                    data = BMI08_SET_BITS_POS_0(data, BMI08_GYRO_RANGE, range);
                    rslt = bmi08g_set_regs(BMI08_REG_GYRO_RANGE, &data, (BMI08_REG_GYRO_RANGE_LENGTH - 2), dev);
                }

                if (rslt == BMI08_OK)
                {
                    dev->delay_us(BMI08_GYRO_SET_CONFIG_DELAY * 1000, dev->intf_ptr_gyro);
                }
            }
        }
        else
        {
            rslt = BMI08_E_INVALID_CONFIG;
        }
    }

    return rslt;
}

int8_t bmi08g_get_power_mode(struct bmi08_dev *dev)
{
    int8_t rslt;
    uint8_t data;

    rslt = null_ptr_check(dev);

    if (rslt == BMI08_OK)
    {
        rslt = bmi08g_get_regs(BMI08_REG_GYRO_LPM1, &data, BMI08_REG_GYRO_LPM_LENGTH, dev);

        if (rslt == BMI08_OK)
        {
            dev->gyro_cfg.power = data;
        }
    }

    return rslt;
}

int8_t bmi08g_set_power_mode(struct bmi08_dev *dev)
{
    int8_t rslt;
    uint8_t power_mode, data;
    uint8_t is_power_switching_mode_valid = TRUE;

    rslt = null_ptr_check(dev);

    if (rslt == BMI08_OK)
    {
        rslt = bmi08g_get_regs(BMI08_REG_GYRO_LPM1, &data, BMI08_REG_GYRO_LPM_LENGTH, dev);

        if (rslt == BMI08_OK)
        {
            power_mode = dev->gyro_cfg.power;

            if ((power_mode == BMI08_GYRO_PM_SUSPEND) && (data == BMI08_GYRO_PM_DEEP_SUSPEND))
            {
                is_power_switching_mode_valid = FALSE;
            }

            if ((power_mode == BMI08_GYRO_PM_DEEP_SUSPEND) && (data == BMI08_GYRO_PM_SUSPEND))
            {
                is_power_switching_mode_valid = FALSE;
            }

            if (is_power_switching_mode_valid)
            {
                rslt = bmi08g_set_regs(BMI08_REG_GYRO_LPM1, &power_mode, BMI08_REG_GYRO_LPM_LENGTH, dev);

                if (rslt == BMI08_OK)
                {
                    dev->delay_us(BMI08_MS_TO_US(BMI08_GYRO_POWER_MODE_CONFIG_DELAY), dev->intf_ptr_gyro);
                }
            }
            else
            {
                rslt = BMI08_E_INVALID_INPUT;
            }
        }
    }

    return rslt;
}

int8_t bmi08g_get_data(struct bmi08_sensor_data *gyro, struct bmi08_dev *dev)
{
    int8_t rslt;
    uint8_t data[6];
    uint8_t lsb, msb;
    uint16_t msblsb;

    rslt = null_ptr_check(dev);

    if ((rslt == BMI08_OK) && (gyro != NULL))
    {
        rslt = bmi08g_get_regs(BMI08_REG_GYRO_X_LSB, data, BMI08_REG_GYRO_X_LSB_LENGTH, dev);

        if (rslt == BMI08_OK)
        {
            lsb = data[0];
            msb = data[1];
            msblsb = (uint16_t)((msb << 8) | lsb);
            gyro->x = (int16_t)msblsb;

            lsb = data[2];
            msb = data[3];
            msblsb = (uint16_t)((msb << 8) | lsb);
            gyro->y = (int16_t)msblsb;

            lsb = data[4];
            msb = data[5];
            msblsb = (uint16_t)((msb << 8) | lsb);
            gyro->z = (int16_t)msblsb;
        }
    }
    else
    {
        rslt = BMI08_E_NULL_PTR;
    }

    return rslt;
}

int8_t bmi08g_set_int_config(const struct bmi08_gyro_int_channel_cfg *int_config, struct bmi08_dev *dev)
{
    int8_t rslt;

    rslt = null_ptr_check(dev);

    if ((rslt == BMI08_OK) && (int_config != NULL))
    {
        switch (int_config->int_type)
        {
            case BMI08_GYRO_INT_DATA_RDY:
                rslt = set_gyro_data_ready_int(int_config, dev);
                break;
            case BMI08_GYRO_INT_FIFO_WM:
            case BMI08_GYRO_INT_FIFO_FULL:
                rslt = set_fifo_int(int_config, dev);
                break;
            default:
                rslt = BMI08_E_INVALID_CONFIG;
                break;
        }
    }
    else
    {
        rslt = BMI08_E_NULL_PTR;
    }

    return rslt;
}

int8_t bmi08g_perform_selftest(struct bmi08_dev *dev)
{
    int8_t rslt;
    uint8_t data = 0, loop_break = 1;

    rslt = null_ptr_check(dev);

    if (rslt == BMI08_OK)
    {
        rslt = set_gyro_selftest(BMI08_ENABLE, dev);

        if (rslt == BMI08_OK)
        {
            while (loop_break)
            {
                rslt = bmi08g_get_regs(BMI08_REG_GYRO_SELF_TEST, &data, BMI08_REG_GYRO_SELF_TEST_LENGTH, dev);

                if (rslt == BMI08_OK)
                {
                    data = BMI08_GET_BITS(data, BMI08_GYRO_SELF_TEST_RDY);

                    if (data)
                    {
                        loop_break = 0;
                    }
                }
                else
                {
                    loop_break = 0;
                }
            }

            if (rslt == BMI08_OK)
            {
                rslt = bmi08g_get_regs(BMI08_REG_GYRO_SELF_TEST, &data, BMI08_REG_GYRO_SELF_TEST_LENGTH, dev);

                if (rslt == BMI08_OK)
                {
                    data = BMI08_GET_BITS(data, BMI08_GYRO_SELF_TEST_RESULT);

                    rslt = bmi08g_soft_reset(dev);

                    if (rslt == BMI08_OK)
                    {
                        rslt = (int8_t)data;
                    }
                }
            }
        }
    }

    return rslt;
}

int8_t bmi08g_get_data_int_status(uint8_t *int_status, struct bmi08_dev *dev)
{
    int8_t rslt;
    uint8_t status = 0;

    if (int_status != NULL)
    {
        rslt = bmi08g_get_regs(BMI08_REG_GYRO_INT_STAT_1, &status, BMI08_REG_GYRO_INT_STAT_LENGTH, dev);
        if (rslt == BMI08_OK)
        {
            (*int_status) = status;
        }
    }
    else
    {
        rslt = BMI08_E_NULL_PTR;
    }

    return rslt;
}

int8_t bmi08g_get_fifo_overrun(uint8_t *fifo_overrun, struct bmi08_dev *dev)
{
    int8_t rslt;
    uint8_t reg_data = 0;

    if (fifo_overrun != NULL)
    {
        rslt = bmi08g_get_regs(BMI08_REG_GYRO_FIFO_STATUS, &reg_data, BMI08_FIFO_STATUS_LENGTH, dev);

        if (rslt == BMI08_OK)
        {
            *fifo_overrun = BMI08_GET_BITS(reg_data, BMI08_GYRO_FIFO_OVERRUN);
        }
    }
    else
    {
        rslt = BMI08_E_NULL_PTR;
    }

    return rslt;
}

int8_t bmi08g_get_fifo_config(struct bmi08_gyr_fifo_config *fifo_conf, struct bmi08_dev *dev)
{
    int8_t rslt;
    uint8_t fifo_config[2] = { 0 };
    uint8_t reg_data = 0;

    if (fifo_conf != NULL)
    {
        rslt = bmi08g_get_regs(BMI08_REG_GYRO_FIFO_CONFIG0, fifo_config, BMI08_FIFO_CONFIG_LENGTH, dev);

        if (rslt == BMI08_OK)
        {
            rslt = bmi08g_get_regs(BMI08_REG_GYRO_FIFO_STATUS, &reg_data, BMI08_FIFO_STATUS_LENGTH, dev);

            if (rslt == BMI08_OK)
            {
                fifo_conf->tag = BMI08_GET_BITS(fifo_config[0], BMI08_GYRO_FIFO_TAG);
                fifo_conf->wm_level = BMI08_GET_BITS_POS_0(fifo_config[0], BMI08_GYRO_FIFO_WM_LEVEL);
                fifo_conf->mode = BMI08_GET_BITS(fifo_config[1], BMI08_GYRO_FIFO_MODE);
                fifo_conf->data_select = BMI08_GET_BITS_POS_0(fifo_config[1], BMI08_GYRO_FIFO_DATA_SELECT);
                fifo_conf->frame_count = BMI08_GET_BITS_POS_0(reg_data, BMI08_GYRO_FIFO_FRAME_COUNT);
            }
        }
    }
    else
    {
        rslt = BMI08_E_NULL_PTR;
    }

    return rslt;
}

int8_t bmi08g_get_fifo_ext_int_sync(struct bmi08_gyro_fifo_ext_int *fifo_config, struct bmi08_dev *dev)
{
    int8_t rslt;
    uint8_t fifo_reg_data = 0;

    if (fifo_config != NULL)
    {
        rslt = bmi08g_get_regs(BMI08_REG_GYRO_FIFO_EXT_INT_S, &fifo_reg_data, BMI08_FIFO_LENGTH_MSB_BYTE, dev);

        if (rslt == BMI08_OK)
        {
            fifo_config->ext_fifo_sync_en = BMI08_GET_BITS(fifo_reg_data, BMI08_GYRO_FIFO_EXT_INT_EN);
            fifo_config->ext_fifo_ext_int_sync_src = BMI08_GET_BITS(fifo_reg_data, BMI08_GYRO_FIFO_EXT_INT_SYNC);
        }
    }
    else
    {
        rslt = BMI08_E_NULL_PTR;
    }

    return rslt;
}

int8_t bmi08g_set_fifo_ext_int_sync(const struct bmi08_gyro_fifo_ext_int *fifo_config, struct bmi08_dev *dev)
{
    int8_t rslt;
    uint8_t fifo_reg_data = 0;

    if (fifo_config != NULL)
    {
        rslt = bmi08g_get_regs(BMI08_REG_GYRO_FIFO_EXT_INT_S, &fifo_reg_data, BMI08_FIFO_LENGTH_MSB_BYTE, dev);

        if (rslt == BMI08_OK)
        {
            fifo_reg_data = BMI08_SET_BITS(fifo_reg_data, BMI08_GYRO_FIFO_EXT_INT_EN, fifo_config->ext_fifo_sync_en);
            fifo_reg_data = BMI08_SET_BITS(fifo_reg_data,
                                           BMI08_GYRO_FIFO_EXT_INT_SYNC,
                                           fifo_config->ext_fifo_ext_int_sync_src);

            rslt = bmi08g_set_regs(BMI08_REG_GYRO_FIFO_EXT_INT_S, &fifo_reg_data, 1, dev);
        }
    }
    else
    {
        rslt = BMI08_E_NULL_PTR;
    }

    return rslt;
}

int8_t bmi08g_set_fifo_config(const struct bmi08_gyr_fifo_config *fifo_conf, struct bmi08_dev *dev)
{
    int8_t rslt;
    uint8_t fifo_config[2] = { 0 };

    if (fifo_conf != NULL)
    {
        rslt = bmi08g_get_regs(BMI08_REG_GYRO_FIFO_CONFIG0, fifo_config, BMI08_FIFO_CONFIG_LENGTH, dev);

        if (rslt == BMI08_OK)
        {
            fifo_config[0] = BMI08_SET_BITS(fifo_config[0], BMI08_GYRO_FIFO_TAG, fifo_conf->tag);
            fifo_config[0] = BMI08_SET_BITS_POS_0(fifo_config[0], BMI08_GYRO_FIFO_WM_LEVEL, fifo_conf->wm_level);
            fifo_config[1] = BMI08_SET_BITS_POS_0(fifo_config[1], BMI08_GYRO_FIFO_DATA_SELECT, fifo_conf->data_select);
            fifo_config[1] = BMI08_SET_BITS(fifo_config[1], BMI08_GYRO_FIFO_MODE, fifo_conf->mode);

            rslt = bmi08g_set_regs(BMI08_REG_GYRO_FIFO_CONFIG0, fifo_config, BMI08_FIFO_CONFIG_LENGTH, dev);
        }
    }
    else
    {
        rslt = BMI08_E_NULL_PTR;
    }

    return rslt;
}

int8_t bmi08g_get_fifo_length(const struct bmi08_gyr_fifo_config *fifo_config, struct bmi08_fifo_frame *fifo)
{
    int8_t rslt = BMI08_OK;
    uint16_t fifo_data_byte_count = 0;

    if ((fifo != NULL) && (fifo_config != NULL))
    {
        if (fifo_config->data_select == BMI08_GYRO_FIFO_XYZ_AXIS_ENABLED)
        {
            get_fifo_data_length(fifo_config, BMI08_GYRO_FIFO_XYZ_AXIS_FRAME_SIZE, &fifo_data_byte_count);
        }
        else
        {
            get_fifo_data_length(fifo_config, BMI08_GYRO_FIFO_SINGLE_AXIS_FRAME_SIZE, &fifo_data_byte_count);
        }

        if (fifo->length > fifo_data_byte_count)
        {
            fifo->length = fifo_data_byte_count;
        }
    }
    else
    {
        rslt = BMI08_E_NULL_PTR;
    }

    return rslt;
}

int8_t bmi08g_read_fifo_data(const struct bmi08_fifo_frame *fifo, struct bmi08_dev *dev)
{
    int8_t rslt = BMI08_OK;

    if (fifo != NULL)
    {
        rslt = bmi08g_get_regs(BMI08_REG_GYRO_FIFO_DATA, fifo->data, fifo->length, dev);
    }
    else
    {
        rslt = BMI08_E_NULL_PTR;
    }

    return rslt;
}

void bmi08g_extract_gyro(struct bmi08_sensor_data *gyro_data,
                         const uint16_t *gyro_length,
                         const struct bmi08_gyr_fifo_config *fifo_conf,
                         const struct bmi08_fifo_frame *fifo)
{
    uint16_t data_index = 0;
    uint16_t gyro_index = 0;
    uint16_t data_read_length = 0;

    parse_fifo_gyro_len(&data_read_length, gyro_length, fifo_conf, fifo);

    for (; data_index < data_read_length;)
    {
        unpack_gyro_data(&gyro_data[gyro_index], &data_index, fifo_conf, fifo);
        gyro_index++;
    }
}

int8_t bmi08g_enable_watermark(uint8_t enable, struct bmi08_dev *dev)
{
    int8_t rslt;
    uint8_t reg_data;

    if (enable)
    {
        reg_data = BMI08_GYRO_FIFO_WM_ENABLE_VAL;
        rslt = bmi08g_set_regs(BMI08_REG_GYRO_FIFO_WM_ENABLE, &reg_data, (BMI08_FIFO_WTM_LENGTH - 1), dev);
    }
    else
    {
        reg_data = BMI08_GYRO_FIFO_WM_DISABLE_VAL;
        rslt = bmi08g_set_regs(BMI08_REG_GYRO_FIFO_WM_ENABLE, &reg_data, (BMI08_FIFO_WTM_LENGTH - 1), dev);
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

static int8_t get_regs(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, struct bmi08_dev *dev)
{
    int8_t rslt = BMI08_OK;

    if (dev->intf == BMI08_SPI_INTF)
    {
        reg_addr = (reg_addr | BMI08_SPI_RD_MASK);
    }

    dev->intf_rslt = dev->read(reg_addr, reg_data, len, dev->intf_ptr_gyro);

    if (dev->intf_rslt != BMI08_INTF_RET_SUCCESS)
    {
        rslt = BMI08_E_COM_FAIL;
    }

    return rslt;
}

static int8_t set_regs(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, struct bmi08_dev *dev)
{
    int8_t rslt = BMI08_OK;
    uint8_t count = 0;

    if (dev->intf == BMI08_SPI_INTF)
    {
        reg_addr = (reg_addr & BMI08_SPI_WR_MASK);
    }

    if (len == 1)
    {
        dev->intf_rslt = dev->write(reg_addr, reg_data, len, dev->intf_ptr_gyro);

        if (dev->intf_rslt != BMI08_INTF_RET_SUCCESS)
        {
            rslt = BMI08_E_COM_FAIL;
        }
    }

    if (len > 1)
    {
        for (count = 0; count < len; count++)
        {
            dev->intf_rslt = dev->write(reg_addr, &reg_data[count], BMI08_GYRO_DATA_LENGTH, dev->intf_ptr_gyro);
            reg_addr++;

            if (dev->intf_rslt != BMI08_INTF_RET_SUCCESS)
            {
                rslt = BMI08_E_COM_FAIL;
                break;
            }
        }
    }

    return rslt;
}

static int8_t set_gyro_data_ready_int(const struct bmi08_gyro_int_channel_cfg *int_config, struct bmi08_dev *dev)
{
    int8_t rslt;
    uint8_t conf, data[2] = { 0 };

    rslt = get_regs(BMI08_REG_GYRO_INT3_INT4_IO_MAP, &data[0], BMI08_REG_ACCEL_INT_MAP_CFG_LENGTH, dev);

    if (rslt == BMI08_OK)
    {
        conf = int_config->int_pin_cfg.enable_int_pin;

        switch (int_config->int_channel)
        {
            case BMI08_INT_CHANNEL_3:
                data[0] = BMI08_SET_BITS_POS_0(data[0], BMI08_GYRO_INT3_MAP, conf);
                break;

            case BMI08_INT_CHANNEL_4:
                data[0] = BMI08_SET_BITS(data[0], BMI08_GYRO_INT4_MAP, conf);
                break;

            default:
                rslt = BMI08_E_INVALID_INPUT;
                break;
        }

        if (rslt == BMI08_OK)
        {
            if (data[0] & BMI08_GYRO_MAP_DRDY_TO_BOTH_INT3_INT4)
            {
                data[1] = BMI08_GYRO_DRDY_INT_ENABLE_VAL;
            }
            else
            {
                data[1] = BMI08_GYRO_DRDY_INT_DISABLE_VAL;
            }

            rslt = bmi08g_set_regs(BMI08_REG_GYRO_INT3_INT4_IO_MAP, &data[0], BMI08_REG_ACCEL_INT_MAP_CFG_LENGTH, dev);

            if (rslt == BMI08_OK)
            {
                rslt = set_int_pin_config(int_config, dev);

                if (rslt == BMI08_OK)
                {
                    rslt = bmi08g_set_regs(BMI08_REG_GYRO_INT_CTRL, &data[1], BMI08_REG_GYRO_INT_CTRL_LENGTH, dev);
                }
            }
        }
    }

    return rslt;
}

static int8_t set_fifo_int(const struct bmi08_gyro_int_channel_cfg *int_config, struct bmi08_dev *dev)
{
    int8_t rslt;
    uint8_t conf, data[2] = { 0 };

    rslt = get_regs(BMI08_REG_GYRO_INT3_INT4_IO_MAP, &data[0], BMI08_REG_ACCEL_INT_MAP_CFG_LENGTH, dev);

    if (rslt == BMI08_OK)
    {
        conf = int_config->int_pin_cfg.enable_int_pin;

        switch (int_config->int_channel)
        {
            case BMI08_INT_CHANNEL_3:
                data[0] = BMI08_SET_BITS(data[0], BMI08_GYRO_FIFO_INT3, conf);
                break;

            case BMI08_INT_CHANNEL_4:
                data[0] = BMI08_SET_BITS(data[0], BMI08_GYRO_FIFO_INT4, conf);
                break;

            default:
                rslt = BMI08_E_INVALID_INPUT;
                break;
        }

        if (rslt == BMI08_OK)
        {
            if (data[0] & BMI08_GYRO_MAP_FIFO_BOTH_INT3_INT4)
            {
                data[1] = BMI08_GYRO_FIFO_INT_ENABLE_VAL;
            }
            else
            {
                data[1] = BMI08_GYRO_FIFO_INT_DISABLE_VAL;
            }

            rslt = bmi08g_set_regs(BMI08_REG_GYRO_INT3_INT4_IO_MAP, &data[0], BMI08_REG_ACCEL_INT_MAP_CFG_LENGTH, dev);

            if (rslt == BMI08_OK)
            {
                rslt = set_int_pin_config(int_config, dev);

                if (rslt == BMI08_OK)
                {
                    rslt = bmi08g_set_regs(BMI08_REG_GYRO_INT_CTRL, &data[1], BMI08_REG_GYRO_INT_CTRL_LENGTH, dev);
                }
            }
        }
    }

    return rslt;
}

static int8_t set_int_pin_config(const struct bmi08_gyro_int_channel_cfg *int_config, struct bmi08_dev *dev)
{
    int8_t rslt;
    uint8_t data;

    rslt = get_regs(BMI08_REG_GYRO_INT3_INT4_IO_CONF, &data, BMI08_REG_GYRO_INT_IO_CONF_LENGTH, dev);

    if (rslt == BMI08_OK)
    {
        switch (int_config->int_channel)
        {
            case BMI08_INT_CHANNEL_3:
                data = BMI08_SET_BITS_POS_0(data, BMI08_GYRO_INT3_LVL, int_config->int_pin_cfg.lvl);
                data = BMI08_SET_BITS(data, BMI08_GYRO_INT3_OD, int_config->int_pin_cfg.output_mode);
                break;

            case BMI08_INT_CHANNEL_4:
                data = BMI08_SET_BITS(data, BMI08_GYRO_INT4_LVL, int_config->int_pin_cfg.lvl);
                data = BMI08_SET_BITS(data, BMI08_GYRO_INT4_OD, int_config->int_pin_cfg.output_mode);
                break;

            default:
                break;
        }

        rslt = bmi08g_set_regs(BMI08_REG_GYRO_INT3_INT4_IO_CONF, &data, BMI08_REG_GYRO_INT_IO_CONF_LENGTH, dev);
    }

    return rslt;
}

static int8_t set_gyro_selftest(uint8_t selftest, struct bmi08_dev *dev)
{
    int8_t rslt;
    uint8_t data = 0;

    if ((selftest == BMI08_ENABLE) || (selftest == BMI08_DISABLE))
    {
        rslt = get_regs(BMI08_REG_GYRO_SELF_TEST, &data, BMI08_REG_GYRO_SELF_TEST_LENGTH, dev);

        if (rslt == BMI08_OK)
        {
            data = BMI08_SET_BITS_POS_0(data, BMI08_GYRO_SELF_TEST_EN, selftest);
            rslt = bmi08g_set_regs(BMI08_REG_GYRO_SELF_TEST, &data, BMI08_REG_GYRO_SELF_TEST_LENGTH, dev);
        }
    }
    else
    {
        rslt = BMI08_E_INVALID_INPUT;
    }

    return rslt;
}

static void get_fifo_data_length(const struct bmi08_gyr_fifo_config *fifo, int8_t frame_size, uint16_t *fifo_data_byte)
{
    if (fifo->tag)
    {
        *fifo_data_byte = (uint16_t)(fifo->frame_count * (frame_size + 2));
    }
    else
    {
        *fifo_data_byte = (uint16_t)(fifo->frame_count * frame_size);
    }
}

static void parse_fifo_gyro_len(uint16_t *len,
                                const uint16_t *gyr_count,
                                const struct bmi08_gyr_fifo_config *fifo_conf,
                                const struct bmi08_fifo_frame *fifo)
{
    if (fifo_conf->tag == 0)
    {
        *len = fifo->length;
    }
    else if ((fifo_conf->tag == 1))
    {
        if (fifo_conf->data_select == BMI08_GYRO_FIFO_XYZ_AXIS_ENABLED)
        {
            *len = (uint16_t)((*gyr_count) * BMI08_GYRO_FIFO_XYZ_AXIS_FRAME_SIZE);
        }
        else
        {
            *len = (uint16_t)((*gyr_count) * BMI08_GYRO_FIFO_SINGLE_AXIS_FRAME_SIZE);
        }
    }
}

static void unpack_gyro_data(struct bmi08_sensor_data *gyro,
                             uint16_t *data_index,
                             const struct bmi08_gyr_fifo_config *fifo_conf,
                             const struct bmi08_fifo_frame *fifo)
{
    uint16_t data_lsb;
    uint16_t data_msb;
    uint16_t idx;

    idx = *data_index;

    data_lsb = fifo->data[idx++];
    data_msb = fifo->data[idx++];
    gyro->x = (int16_t)((data_msb << 8) | data_lsb);

    data_lsb = fifo->data[idx++];
    data_msb = fifo->data[idx++];
    gyro->y = (int16_t)((data_msb << 8) | data_lsb);

    data_lsb = fifo->data[idx++];
    data_msb = fifo->data[idx++];
    gyro->z = (int16_t)((data_msb << 8) | data_lsb);

    if (fifo_conf->tag == 1)
    {
        idx += 2;
    }

    *data_index = idx;
}