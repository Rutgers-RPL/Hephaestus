#include "device-init.h"

#include "main.h"

#include "sensor.h"
#include "flash.h"
#include "spi.h"

#include "base_task.h"
#include "flash_task.h"
#include "sensor_task.h"
#include "bmi088.h"
#include "bmp581.h"
#include "gd5f1gq5xe.h"

#include "log.h"

#include <array>
#include <optional>

extern CRC_HandleTypeDef hcrc;
extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;
extern SPI_HandleTypeDef hspi3;

namespace {
  // Peripherals
  Common::SPI accel(&hspi1, IMU2_ACC_CS_GPIO_Port, IMU2_ACC_CS_Pin);
  Common::SPI gyro(&hspi1, IMU2_GYRO_CS_GPIO_Port, IMU2_GYRO_CS_Pin);
  Common::SPI bmp(&hspi2, BAR1_CS_GPIO_Port, BAR1_CS_Pin);
  Common::SPI flash(&hspi3, FLASH_CS_GPIO_Port, FLASH_CS_Pin);

  // Sensors and Flash
  enum Sensors { BMP581, BMI088, NUM_SENSORS };
  Common::GD5F1GQ5XE flash_dev{flash};
  Common::BMP581 bmp581_dev{bmp};
  Common::BMI088 bmi088_dev{accel, gyro};
  std::array<Common::Sensor*, NUM_SENSORS> sensors{&bmp581_dev, &bmi088_dev};

  // Runtime State
  int32_t fs_size = 0;
  uint32_t boot_count = 0;
  uint32_t file_size = 0;
  lfs_file_t packet_file;
  Common::DoubleBuffer<Common::Packet> packet;

  // Tasks
  Common::FlashTask flash_task(flash_dev, packet_file, packet);
  Common::SensorTask sensor_task(sensors, hcrc, packet);
} // end namespace

extern "C" {
  void device_init() {
    // Initialize sensors
    for (auto& sensor : sensors) {
      bool ready = 0;
      for (int c = 0; c < 10; ++c) {
        ready = sensor->init();
        if (ready) break;
        Delay(20000);
      }
      if (!ready) sensor = nullptr; 
    }

    // Initialize flash
    for (int c = 0; c < 3; ++c) {
      if (flash_dev.init()) {
        fs_size = flash_dev.mount();
        if (fs_size >= 0) {
          boot_count = flash_dev.bootcount(false);
          file_size = flash_dev.open(&packet_file, "packets");
        }
        break;
      }
      Delay(5000);
    }
    Common::LOG("Flash: %u (size), %u (boot), %u (packet_size)\r\n", fs_size, boot_count, file_size);

    sensor_task.init("sensor task", 256, 2);
    if (fs_size >= 0) {
      flash_task.init("flash task", 512, 1);
    }
  }

  void device_disable_flash() {
    Common::Task::registry().publish_isr(Common::TaskEvent::FlashDisabled, pdFALSE);
  }
}
