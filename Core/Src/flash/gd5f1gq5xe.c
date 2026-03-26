#include "gd5f1gq5xe.h"

#include "main.h"

extern SPI_HandleTypeDef hspi3;

static void chip_select()
{
	HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET);
}

static void chip_deselect()
{
	HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET);
}

static bool spi_transmit(void *buffer, const size_t size)
{
	return HAL_SPI_Transmit(&hspi3, (uint8_t*) buffer, size, HAL_MAX_DELAY);
}

static bool spi_receive(void *buffer, const size_t size)
{
	return HAL_SPI_Receive(&hspi3, (uint8_t*) buffer, size, HAL_MAX_DELAY);
}

static int write_enable()
{
	uint8_t tx = GD5F_WRITE_ENABLE;
	chip_select();
	if (spi_transmit(&tx, sizeof(tx)) != 0) {
		chip_deselect();
		return 1;
	}
	chip_deselect();
	return 0;
}

int gd5f1gq5xe_read(const uint32_t page, const uint16_t col, void *buffer, const size_t size)
{
	// Read Page Command requires read op code followed by 24-bit address
	uint8_t tx1[] = {
		GD5F_READ_TO_CACHE,
		0x00,
		(page & 0x00FF00) >> 8,
		(page & 0x0000FF)
	};
	chip_select();
	if (spi_transmit(tx1, sizeof(tx1)) != 0) {
		chip_deselect();
		return 1;
	}
	chip_deselect();
	HAL_Delay(1);

	// Read data from cache requires the op code followed by four dummy bits
	// (0) and then a 11 bit address. It also needs dummy bytes after.
	uint8_t tx2[] = {
		GD5F_READ_FROM_CACHE,
		(col & 0x0F00) >> 8,
		(col & 0x00FF),
		0x00
	};
	chip_select();
	if (spi_transmit(tx2, sizeof(tx2)) != 0) {
		chip_deselect();
		return 1;
	};
	if (spi_receive(buffer, size) != 0) {
		chip_deselect();
		return 1;
	}
	chip_deselect();
	return 0;
}

int gd5f1gq5xe_write(const uint32_t page, const uint16_t col, const void *buffer, const size_t size)
{
	if (write_enable() != 0) {
		chip_deselect();
		return 1;
	}

	uint8_t tx1[] = {
		GD5F_PROGRAM_LOAD,
		(col & 0x0F00) >> 8,
		(col & 0x00FF)
	};
	chip_select();
	if (spi_transmit(tx1, sizeof(tx1)) != 0) {
		chip_deselect();
		return 1;
	}
	if (spi_transmit(buffer, size) != 0) {
		chip_deselect();
		return 1;
	};
	chip_deselect();

	uint8_t tx2[] = {
		GD5F_PROGRAM_EXECUTE,
		0x00,
		(page & 0x00FF00) >> 8,
		(page & 0x0000FF)
	};
	chip_select();
	if (spi_transmit(tx2, sizeof(tx2)) != 0) {
		chip_deselect();
		return 1;
	}
	chip_deselect();
	
	HAL_Delay(5);
	return 0;
}

int gd5f1gq5xe_erase(const uint32_t page)
{
	if (write_enable() != 0) {
		chip_deselect();
		return 1;
	}

	uint8_t tx[] = {
		GD5F_ERASE,
		0x00,
		(page & 0x00FF00) >> 8,
		(page & 0x0000FF)
	};
	chip_select();
	if (spi_transmit(tx, sizeof(tx)) != 0) {
		chip_deselect();
		return 1;
	}
	chip_deselect();

	HAL_Delay(25);
	return 0;
}

int gd5f1gq5xe_unlock()
{
	if (write_enable() != 0) {
		chip_deselect();
		return 1;
	}
	uint8_t tx[] = {
		GD5F_SET_FEATURE,
		0xA0,
		0x00,
	};
	chip_select();
	if (spi_transmit(tx, sizeof(tx)) != 0) {
		chip_deselect();
		return 1;
	}
	chip_deselect();

	HAL_Delay(25);
	return 0;
}
