/*
 * imu_board.h
 *
 *  Created on: Nov 5, 2025
 *      Author: Gabriele Lisci and Johnathan Persaud
 */

#ifndef SRC_IMU_BOARD_H_
#define SRC_IMU_BOARD_H_

#include "main.h"
#include "inv_imu_transport.h"

int stm32_read_reg(uint8_t reg, uint8_t *buf, uint32_t len);
int stm32_write_reg(uint8_t reg, const uint8_t *buf, uint32_t len);
void stm32_sleep_us(uint32_t us);

extern inv_imu_transport_t imu_transport;

#endif /* SRC_IMU_BOARD_H_ */
