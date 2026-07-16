/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define BAR1_CS_Pin GPIO_PIN_0
#define BAR1_CS_GPIO_Port GPIOC
#define BAR1_INT_Pin GPIO_PIN_1
#define BAR1_INT_GPIO_Port GPIOC
#define BAR_MISO_Pin GPIO_PIN_2
#define BAR_MISO_GPIO_Port GPIOC
#define BAR_MOSI_Pin GPIO_PIN_3
#define BAR_MOSI_GPIO_Port GPIOC
#define BAR2_CS_Pin GPIO_PIN_1
#define BAR2_CS_GPIO_Port GPIOA
#define BAR2_INT_Pin GPIO_PIN_2
#define BAR2_INT_GPIO_Port GPIOA
#define IMU_SCK_Pin GPIO_PIN_5
#define IMU_SCK_GPIO_Port GPIOA
#define IMU_MISO_Pin GPIO_PIN_6
#define IMU_MISO_GPIO_Port GPIOA
#define IMU_MOSI_Pin GPIO_PIN_7
#define IMU_MOSI_GPIO_Port GPIOA
#define IMU1_INT_Pin GPIO_PIN_4
#define IMU1_INT_GPIO_Port GPIOC
#define IMU1_CS_Pin GPIO_PIN_5
#define IMU1_CS_GPIO_Port GPIOC
#define IMU2_GYRO_INT_Pin GPIO_PIN_0
#define IMU2_GYRO_INT_GPIO_Port GPIOB
#define IMU2_GYRO_CS_Pin GPIO_PIN_1
#define IMU2_GYRO_CS_GPIO_Port GPIOB
#define IMU2_ACC_CS_Pin GPIO_PIN_2
#define IMU2_ACC_CS_GPIO_Port GPIOB
#define IMU2_ACC_INT_Pin GPIO_PIN_10
#define IMU2_ACC_INT_GPIO_Port GPIOB
#define LED_Pin GPIO_PIN_12
#define LED_GPIO_Port GPIOB
#define BUZZER_Pin GPIO_PIN_13
#define BUZZER_GPIO_Port GPIOB
#define PYRO3_Pin GPIO_PIN_15
#define PYRO3_GPIO_Port GPIOB
#define PYRO3_SENSE_Pin GPIO_PIN_6
#define PYRO3_SENSE_GPIO_Port GPIOC
#define BAR_SCK_Pin GPIO_PIN_7
#define BAR_SCK_GPIO_Port GPIOC
#define PYRO1_SENSE_Pin GPIO_PIN_8
#define PYRO1_SENSE_GPIO_Port GPIOC
#define PYRO1_Pin GPIO_PIN_9
#define PYRO1_GPIO_Port GPIOC
#define PYRO2_Pin GPIO_PIN_8
#define PYRO2_GPIO_Port GPIOA
#define PYRO2_SENSE_Pin GPIO_PIN_9
#define PYRO2_SENSE_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define MAG_FLASH_SCK_Pin GPIO_PIN_10
#define MAG_FLASH_SCK_GPIO_Port GPIOC
#define MAG_FLASH_MISO_Pin GPIO_PIN_11
#define MAG_FLASH_MISO_GPIO_Port GPIOC
#define MAG_FLASH_MOSI_Pin GPIO_PIN_12
#define MAG_FLASH_MOSI_GPIO_Port GPIOC
#define MAG_CS_Pin GPIO_PIN_2
#define MAG_CS_GPIO_Port GPIOD
#define MAG_INT_Pin GPIO_PIN_3
#define MAG_INT_GPIO_Port GPIOB
#define FLASH_CS_Pin GPIO_PIN_4
#define FLASH_CS_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
