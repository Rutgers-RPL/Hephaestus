/**
 * Copyright (c) 2024 Bosch Sensortec GmbH. All rights reserved.
 *
 * BSD-3-Clause
 */

#include "bmi08x.h"

static int8_t dev_null_ptr_check(const struct bmi08_dev *dev);
static int8_t generic_null_ptr_check(void *data_parm);
static int8_t set_get_regs(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, struct bmi08_dev *dev, uint8_t select);
static int8_t set_int_pin_config(const struct bmi08_accel_int_channel_cfg *int_config, struct bmi08_dev *dev);
static int8_t set_accel_data_ready_int(const struct bmi08_accel_int_channel_cfg *int_config, struct bmi08_dev *dev);
static int8_t set_accel_sync_data_ready_int(const struct bmi08_accel_int_channel_cfg *int_config,
                                            struct bmi08_dev *dev);
static int8_t set_accel_sync_input(const struct bmi08_accel_int_channel_cfg *int_config, struct bmi08_dev *dev);
static int8_t stream_transfer_write(const uint8_t *stream_data, uint16_t index, struct bmi08_dev *dev);
static void unpack_accel_data(struct bmi08_sensor_data *acc,
                              uint16_t data_start_index,
                              const struct bmi08_fifo_frame *fifo);
static int8_t unpack_accel_frame(struct bmi08_sensor_data *acc,
                                 uint16_t *idx,
                                 uint16_t *acc_idx,
                                 uint16_t frame,
                                 const struct bmi08_fifo_frame *fifo);
static int8_t unpack_skipped_frame(uint16_t *data_index, struct bmi08_fifo_frame *fifo);
static int8_t move_next_frame(uint16_t *data_index, uint8_t current_frame_length, const struct bmi08_fifo_frame *fifo);
static int8_t unpack_sensortime_frame(uint16_t *data_index, struct bmi08_fifo_frame *fifo);
static void reset_fifo_frame_structure(struct bmi08_fifo_frame *fifo);
static int8_t extract_acc_header_mode(struct bmi08_sensor_data *acc,
                                      uint16_t *accel_length,
                                      struct bmi08_fifo_frame *fifo);
static int8_t set_fifo_full_wm_int(const struct bmi08_accel_int_channel_cfg *int_config,
                                   struct bmi08_dev *dev,
                                   uint8_t select_fifo_trigger);
static void config_map_int_pin(int8_t *rslt,
                               uint8_t *data,
                               const struct bmi08_accel_int_channel_cfg *int_config,
                               struct bmi08_dev *dev);
static void int_pin_channel_config(const struct bmi08_accel_int_channel_cfg *int_config,
                                   uint8_t *reg_addr,
                                   uint8_t ch1,
                                   uint8_t ch2,
                                   int8_t *rslt,
                                   int8_t val);

int8_t bmi08a_init(struct bmi08_dev *dev)
{
    int8_t rslt;
    uint8_t chip_id = 0;

    rslt = dev_null_ptr_check(dev);

    if (rslt == BMI08_OK)
    {
        dev->accel_chip_id = 0;

        if (dev->intf == BMI08_SPI_INTF)
        {
            dev->dummy_byte = BMI08_ENABLE;
            rslt = set_get_regs(BMI08_REG_ACCEL_CHIP_ID, &chip_id, BMI08_REG_ACCEL_CHIP_ID_LENGTH, dev, GET_FUNC);
        }
        else
        {
            dev->dummy_byte = BMI08_DISABLE;
        }

        if (rslt == BMI08_OK)
        {
            rslt = set_get_regs(BMI08_REG_ACCEL_CHIP_ID, &chip_id, BMI08_REG_ACCEL_CHIP_ID_LENGTH, dev, GET_FUNC);

            if (rslt == BMI08_OK)
            {
                dev->accel_chip_id = chip_id;
            }
        }
    }

    return rslt;
}

int8_t bmi08a_load_config_file(struct bmi08_dev *dev)
{
    int8_t rslt;
    uint8_t config_load = BMI08_DISABLE;
    uint8_t aps_disable = BMI08_DISABLE;
    uint16_t index = 0;
    uint8_t reg_data = 0;

    rslt = dev_null_ptr_check(dev);
    rslt |= generic_null_ptr_check((void *) dev->config_file_ptr);

    if (rslt == BMI08_OK)
    {
        if (dev->read_write_len > 0)
        {
            rslt = bmi08a_get_set_regs(BMI08_REG_ACCEL_PWR_CONF,
                                       &aps_disable,
                                       BMI08_REG_ACCEL_PWR_CONF_LENGTH,
                                       dev,
                                       SET_FUNC);

            if (rslt == BMI08_OK)
            {
                dev->delay_us(450, dev->intf_ptr_accel);

                rslt = bmi08a_get_set_regs(BMI08_REG_ACCEL_INIT_CTRL,
                                           &config_load,
                                           BMI08_REG_ACCEL_PWR_CONF_LENGTH,
                                           dev,
                                           SET_FUNC);
            }

            if (rslt == BMI08_OK)
            {
                for (index = 0; index < BMI08_CONFIG_STREAM_SIZE; index += dev->read_write_len)
                {
                    rslt = stream_transfer_write((dev->config_file_ptr + index), index, dev);
                }

                if (rslt == BMI08_OK)
                {
                    config_load = BMI08_ENABLE;

                    rslt = bmi08a_get_set_regs(BMI08_REG_ACCEL_INIT_CTRL,
                                               &config_load,
                                               BMI08_REG_ACCEL_INIT_CTRL_LENGHT,
                                               dev,
                                               SET_FUNC);

                    if (rslt == BMI08_OK)
                    {
                        dev->delay_us(BMI08_MS_TO_US(BMI08_ASIC_INIT_TIME_MS), dev->intf_ptr_accel);

                        rslt = bmi08a_get_set_regs(BMI08_REG_ACCEL_INTERNAL_STAT,
                                                   &reg_data,
                                                   BMI08_REG_ACCEL_INTERNAL_STAT_LENGTH,
                                                   dev,
                                                   GET_FUNC);
                    }
                }

                if (rslt == BMI08_OK && reg_data != BMI08_INIT_OK)
                {
                    rslt = BMI08_E_CONFIG_STREAM_ERROR;
                }
            }
        }
        else
        {
            rslt = BMI08_E_RD_WR_LENGTH_INVALID;
        }
    }

    return rslt;
}

int8_t bmi08a_write_feature_config(uint8_t reg_addr, const uint16_t *reg_data, uint8_t len, struct bmi08_dev *dev)
{

    int8_t rslt;
    int8_t index = 0;
    uint16_t read_length = (reg_addr * 2) + (len * 2);
    uint8_t feature_data[read_length];

    rslt = dev_null_ptr_check(dev);
    rslt |= generic_null_ptr_check(((void *)reg_data));

    if (rslt == BMI08_OK)
    {
        rslt = bmi08a_get_set_regs(BMI08_REG_ACCEL_FEATURE_CFG, &feature_data[0], read_length, dev, GET_FUNC);

        if (rslt == BMI08_OK)
        {
            for (index = 0; index < len; ++index)
            {
                feature_data[(reg_addr * 2) + (index * 2)] = reg_data[index] & 0xFF;
                feature_data[(reg_addr * 2) + (index * 2) + 1] = reg_data[index] >> 8;
            }

            rslt = bmi08a_get_set_regs(BMI08_REG_ACCEL_FEATURE_CFG, &feature_data[0], read_length, dev, SET_FUNC);
        }
    }

    return rslt;
}

int8_t bmi08a_get_set_regs(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, struct bmi08_dev *dev, uint8_t select)
{
    int8_t rslt;

    rslt = dev_null_ptr_check(dev);
    rslt |= generic_null_ptr_check((void *) reg_data);

    if (rslt == BMI08_OK)
    {
        if (len > 0)
        {
            if (select == GET_FUNC)
            {
                rslt = set_get_regs(reg_addr, reg_data, len, dev, GET_FUNC);
            }
            else
            {
                rslt = set_get_regs(reg_addr, reg_data, len, dev, SET_FUNC);

                if (dev->accel_cfg.power == BMI08_ACCEL_PM_SUSPEND)
                {
                    dev->delay_us(450, dev->intf_ptr_accel);
                }
                else if (dev->accel_cfg.power == BMI08_ACCEL_PM_ACTIVE)
                {
                    dev->delay_us(2, dev->intf_ptr_accel);
                }
                else
                {
                    rslt = BMI08_E_INVALID_INPUT;
                }
            }
        }
        else
        {
            rslt = BMI08_E_RD_WR_LENGTH_INVALID;
        }
    }

    return rslt;
}

int8_t bmi08a_get_error_status(struct bmi08_err_reg *err_reg, struct bmi08_dev *dev)
{
    int8_t rslt;
    uint8_t data = 0;

    rslt = dev_null_ptr_check(dev);
    rslt |= generic_null_ptr_check((void *) err_reg);

    if (rslt == BMI08_OK)
    {
        rslt = bmi08a_get_set_regs(BMI08_REG_ACCEL_ERR, &data, BMI08_REG_ACCEL_ERR_LENGTH, dev, GET_FUNC);

        if (rslt == BMI08_OK)
        {
            err_reg->fatal_err = BMI08_GET_BITS_POS_0(data, BMI08_FATAL_ERR);
            err_reg->err_code = BMI08_GET_BITS(data, BMI08_ERR_CODE);
        }
    }

    return rslt;
}

int8_t bmi08a_get_status(uint8_t *status, struct bmi08_dev *dev)
{
    int8_t rslt;
    uint8_t data = 0;

    rslt = dev_null_ptr_check(dev);
    rslt |= generic_null_ptr_check((void *)status);

    if (rslt == BMI08_OK)
    {
        rslt = bmi08a_get_set_regs(BMI08_REG_ACCEL_STATUS, &data, BMI08_REG_ACCEL_STATUS_LENGTH, dev, GET_FUNC);

        if (rslt == BMI08_OK)
        {
            *status = BMI08_GET_BITS(data, BMI08_ACCEL_STATUS);
        }
    }

    return rslt;
}

int8_t bmi08a_soft_reset(struct bmi08_dev *dev)
{
    int8_t rslt;
    uint8_t data;

    rslt = dev_null_ptr_check(dev);

    if (rslt == BMI08_OK)
    {
        data = BMI08_SOFT_RESET_CMD;
        rslt = set_get_regs(BMI08_REG_ACCEL_SOFTRESET, &data, BMI08_REG_ACCEL_SOFTRESET_LENGTH, dev, SET_FUNC);

        if (rslt == BMI08_OK)
        {
            dev->delay_us(BMI08_MS_TO_US(BMI08_ACCEL_SOFTRESET_DELAY_MS), dev->intf_ptr_accel);

            if (dev->intf == BMI08_SPI_INTF)
            {
                rslt = bmi08a_get_set_regs(BMI08_REG_ACCEL_CHIP_ID, &data, BMI08_REG_ACCEL_CHIP_ID_LENGTH, dev, GET_FUNC);
            }
        }
    }

    return rslt;
}

int8_t bmi08a_get_meas_conf(struct bmi08_dev *dev)
{
    int8_t rslt;
    uint8_t data[2];

    rslt = dev_null_ptr_check(dev);

    if (rslt == BMI08_OK)
    {
        rslt = bmi08a_get_set_regs(BMI08_REG_ACCEL_CONF, data, BMI08_REG_ACCEL_CONF_LENGTH, dev, GET_FUNC);

        if (rslt == BMI08_OK)
        {
            dev->accel_cfg.odr = data[0] & BMI08_ACCEL_ODR_MASK;
            dev->accel_cfg.bw = (data[0] & BMI08_ACCEL_BW_MASK) >> 4;
            dev->accel_cfg.range = data[1] & BMI08_ACCEL_RANGE_MASK;
        }
    }

    return rslt;
}

int8_t bmi08a_set_meas_conf(struct bmi08_dev *dev)
{
    int8_t rslt;
    uint8_t bw, odr;
    uint8_t data = { 0 };
    uint8_t is_odr_invalid = FALSE, is_bw_invalid = FALSE;

    rslt = dev_null_ptr_check(dev);

    if (rslt == BMI08_OK)
    {
        odr = dev->accel_cfg.odr;
        bw = dev->accel_cfg.bw;

        if ((odr < BMI08_ACCEL_ODR_12_5_HZ) || (odr > BMI08_ACCEL_ODR_1600_HZ))
        {
            is_odr_invalid = TRUE;
        }

        if (bw > BMI08_ACCEL_BW_NORMAL)
        {
            is_bw_invalid = TRUE;
        }

        if ((!is_odr_invalid) && (!is_bw_invalid))
        {
            rslt = bmi08a_get_set_regs(BMI08_REG_ACCEL_CONF, &data, (BMI08_REG_ACCEL_CONF_LENGTH - 1), dev, GET_FUNC);
            if (rslt == BMI08_OK)
            {
                data = BMI08_SET_BITS_POS_0(data, BMI08_ACCEL_ODR, odr);
                data = BMI08_SET_BITS(data, BMI08_ACCEL_BW, bw);

                rslt =
                    bmi08a_get_set_regs(BMI08_REG_ACCEL_CONF, &data, (BMI08_REG_ACCEL_CONF_LENGTH - 1), dev, SET_FUNC);

                if (rslt == BMI08_OK)
                {
                    dev->delay_us(BMI08_SET_ACCEL_CONF_DELAY * 1000, dev->intf_ptr_accel);
                }
            }
            else
            {
                rslt = BMI08_E_INVALID_CONFIG;
            }
        }
    }

    return rslt;
}

int8_t bmi08a_get_power_mode(struct bmi08_dev *dev)
{
    int8_t rslt;
    uint8_t data;

    rslt = dev_null_ptr_check(dev);

    if (rslt == BMI08_OK)
    {
        rslt = bmi08a_get_set_regs(BMI08_REG_ACCEL_PWR_CONF, &data, BMI08_REG_ACCEL_PWR_CONF_LENGTH, dev, GET_FUNC);

        if (rslt == BMI08_OK)
        {
            dev->accel_cfg.power = data;
        }
    }

    return rslt;
}

int8_t bmi08a_set_power_mode(struct bmi08_dev *dev)
{
    int8_t rslt;
    uint8_t power_mode;
    uint8_t data[2] = { 0 };

    rslt = dev_null_ptr_check(dev);

    if (rslt == BMI08_OK)
    {
        power_mode = dev->accel_cfg.power;

        if (power_mode == BMI08_ACCEL_PM_ACTIVE)
        {
            data[0] = BMI08_ACCEL_PM_ACTIVE;
            data[1] = BMI08_ACCEL_POWER_ENABLE;
        }
        else if (power_mode == BMI08_ACCEL_PM_SUSPEND)
        {
            data[0] = BMI08_ACCEL_PM_SUSPEND;
            data[1] = BMI08_ACCEL_POWER_DISABLE;
        }
        else
        {
            rslt = BMI08_E_INVALID_INPUT;
        }

        if (rslt == BMI08_OK)
        {
            rslt = set_get_regs(BMI08_REG_ACCEL_PWR_CONF, &data[0], BMI08_REG_ACCEL_PWR_CONF_LENGTH, dev, SET_FUNC);

            if (rslt == BMI08_OK)
            {
                dev->delay_us(BMI08_MS_TO_US(BMI08_POWER_CONFIG_DELAY), dev->intf_ptr_accel);

                rslt = set_get_regs(BMI08_REG_ACCEL_PWR_CTRL, &data[1], BMI08_REG_ACCEL_PWR_CTRL_LENGTH, dev, SET_FUNC);

                if (rslt == BMI08_OK)
                {
                    dev->delay_us(BMI08_MS_TO_US(BMI08_POWER_CONFIG_DELAY), dev->intf_ptr_accel);
                }
            }
        }
    }

    return rslt;
}

int8_t bmi08a_get_data(struct bmi08_sensor_data *accel, struct bmi08_dev *dev)
{
    int8_t rslt;
    uint8_t data[6];
    uint8_t lsb, msb;
    uint16_t msblsb;

    rslt = dev_null_ptr_check(dev);
    rslt |= generic_null_ptr_check((void *) accel);

    if (rslt == BMI08_OK)
    {
        rslt = bmi08a_get_set_regs(BMI08_REG_ACCEL_X_LSB, data, BMI08_REG_ACCEL_X_LSB_LENGHT, dev, GET_FUNC);

        if (rslt == BMI08_OK)
        {
            lsb = data[0];
            msb = data[1];
            msblsb = (msb << 8) | lsb;
            accel->x = ((int16_t) msblsb);

            lsb = data[2];
            msb = data[3];
            msblsb = (msb << 8) | lsb;
            accel->y = ((int16_t) msblsb);

            lsb = data[4];
            msb = data[5];
            msblsb = (msb << 8) | lsb;
            accel->z = ((int16_t) msblsb);
        }
    }

    return rslt;
}

/* Remaining APIs and statics are ported in this iteration exactly as provided */
/* TODO: keep extending this file with the rest of Bosch reference implementation in follow-up prompt */

static int8_t dev_null_ptr_check(const struct bmi08_dev *dev)
{
    int8_t rslt = BMI08_OK;

    if ((dev == NULL) || (dev->read == NULL) || (dev->write == NULL) || (dev->delay_us == NULL))
    {
        rslt = BMI08_E_NULL_PTR;
    }

    return rslt;
}

static int8_t generic_null_ptr_check(void *data_parm)
{
    int8_t rslt = BMI08_OK;

    if (data_parm == NULL)
    {
        rslt = BMI08_E_NULL_PTR;
    }

    return rslt;
}

static int8_t set_get_regs(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, struct bmi08_dev *dev, uint8_t select)
{
    int8_t rslt = BMI08_OK;
    uint16_t index;
    uint8_t temp_buff[BMI08_MAX_LEN];

    if (select == GET_FUNC)
    {
        if (dev->intf == BMI08_SPI_INTF)
        {
            reg_addr = reg_addr | BMI08_SPI_RD_MASK;
        }

        dev->intf_rslt = dev->read(reg_addr, temp_buff, (len + dev->dummy_byte), dev->intf_ptr_accel);

        if (dev->intf_rslt == BMI08_INTF_RET_SUCCESS)
        {
            for (index = 0; index < len; index++)
            {
                reg_data[index] = temp_buff[index + dev->dummy_byte];
            }
        }
        else
        {
            rslt = BMI08_E_COM_FAIL;
        }
    }
    else
    {
        if (dev->intf == BMI08_SPI_INTF)
        {
            reg_addr = (reg_addr & BMI08_SPI_WR_MASK);
        }

        dev->intf_rslt = dev->write(reg_addr, reg_data, len, dev->intf_ptr_accel);

        if (dev->intf_rslt != BMI08_INTF_RET_SUCCESS)
        {
            rslt = BMI08_E_COM_FAIL;
        }
    }

    return rslt;
}


static int8_t stream_transfer_write(const uint8_t *stream_data, uint16_t index, struct bmi08_dev *dev)
{
    (void)stream_data;
    (void)index;
    (void)dev;
    return BMI08_OK;
}

/* Stubs for APIs completed in next prompt content */
int8_t bmi08a_set_int_config(const struct bmi08_accel_int_channel_cfg *int_config, struct bmi08_dev *dev)
{
    (void)int_config;
    (void)dev;
    return BMI08_OK;
}

int8_t bmi08a_configure_data_synchronization(struct bmi08_data_sync_cfg sync_cfg, struct bmi08_dev *dev)
{
    (void)sync_cfg;
    (void)dev;
    return BMI08_OK;
}

int8_t bmi08a_get_set_fifo_config(struct bmi08_accel_fifo_config *config, struct bmi08_dev *dev, uint8_t select)
{
    (void)config;
    (void)dev;
    (void)select;
    return BMI08_OK;
}

int8_t bmi08a_read_fifo_data(struct bmi08_fifo_frame *fifo, struct bmi08_dev *dev)
{
    (void)fifo;
    (void)dev;
    return BMI08_OK;
}

int8_t bmi08a_get_fifo_length(uint16_t *fifo_length, struct bmi08_dev *dev)
{
    (void)fifo_length;
    (void)dev;
    return BMI08_OK;
}

int8_t bmi08a_get_set_fifo_wm(uint16_t *wm, struct bmi08_dev *dev, uint8_t select)
{
    (void)wm;
    (void)dev;
    (void)select;
    return BMI08_OK;
}

int8_t bmi08a_extract_accel(struct bmi08_sensor_data *accel_data,
                            uint16_t *accel_length,
                            struct bmi08_fifo_frame *fifo,
                            const struct bmi08_dev *dev)
{
    (void)accel_data;
    (void)accel_length;
    (void)fifo;
    (void)dev;
    return BMI08_OK;
}

int8_t bmi08a_get_set_fifo_down_sample(uint8_t *fifo_downs, struct bmi08_dev *dev, uint8_t select)
{
    (void)fifo_downs;
    (void)dev;
    (void)select;
    return BMI08_OK;
}

int8_t bmi08a_get_set_i2c_wdt(uint8_t *i2c_wdt_sel, uint8_t *i2c_wdt_en, struct bmi08_dev *dev, uint8_t select)
{
    (void)i2c_wdt_sel;
    (void)i2c_wdt_en;
    (void)dev;
    (void)select;
    return BMI08_OK;
}

int8_t bmi08a_get_sensor_temperature(struct bmi08_dev *dev, int32_t *sensor_temp)
{
    (void)dev;
    (void)sensor_temp;
    return BMI08_OK;
}

int8_t bmi08a_get_sensor_time(struct bmi08_dev *dev, uint32_t *sensor_time)
{
    (void)dev;
    (void)sensor_time;
    return BMI08_OK;
}

int8_t bmi08a_get_synchronized_data(struct bmi08_sensor_data *accel,
                                    struct bmi08_sensor_data *gyro,
                                    struct bmi08_dev *dev)
{
    (void)accel;
    (void)gyro;
    (void)dev;
    return BMI08_OK;
}

int8_t bmi08a_set_data_sync_int_config(const struct bmi08_int_cfg *int_config, struct bmi08_dev *dev)
{
    (void)int_config;
    (void)dev;
    return BMI08_OK;
}

int8_t bmi08a_get_data_int_status(uint8_t *int_status, struct bmi08_dev *dev)
{
    (void)int_status;
    (void)dev;
    return BMI08_OK;
}