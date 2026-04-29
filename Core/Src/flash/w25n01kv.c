#include "w25n01kv.h"

#include "main.h"

#include <stddef.h>
#include <stdint.h>

extern SPI_HandleTypeDef hspi4;

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
	return HAL_SPI_Transmit(&hspi4, (uint8_t*) buffer, size, HAL_MAX_DELAY) == HAL_OK;
}

static bool spi_receive(void *buffer, const size_t size)
{
	return HAL_SPI_Receive(&hspi4, (uint8_t*) buffer, size, HAL_MAX_DELAY) == HAL_OK;
}

static int w25n01kv_write_enable()
{
	uint8_t tx = W25N_CMD_WRITE_ENABLE;
	chip_select();
	if (!spi_transmit(&tx, sizeof(tx))) {
		chip_deselect();
		return 1;
	}
	chip_deselect();
	return 0;
}

static int w25n01kv_write_disable()
{
	uint8_t tx = W25N_CMD_WRITE_DISABLE;
	chip_select();
	if (!spi_transmit(&tx, sizeof(tx))) {
		chip_deselect();
		return 1;
	}
	chip_deselect();
	return 0;
}

int w25n01kv_read(const uint32_t page, const uint16_t col, void *buffer, const size_t size)
{
	// Read Page Command requires read op code followed by 24-bit address
	uint8_t tx1[] = {
		W25N_CMD_PAGE_DATA_READ,
		(page & 0xFF0000) >> 16,
		(page & 0x00FF00) >> 8,
		(page & 0x0000FF)
	};
	chip_select();
	if (!spi_transmit(tx1, sizeof(tx1))) {
		chip_deselect();
		return 1;
	}
	chip_deselect();
	HAL_Delay(1);

	// Read Data Command requires read op code followed by 16-bit address
	// and a 8-bit dummy clock, called after read page in order to actually
	// get the data. The 16-bit address refers to the column
	uint8_t tx2[] = {
		W25N_CMD_READ_FROM_BUFFER,
		(col & 0xFF00) >> 8,
		(col & 0x00FF),
		0xFF
	};
	chip_select();
	if (!spi_transmit(tx2, sizeof(tx2))) {
		chip_deselect();
		return 1;
	};
	if (!spi_receive(buffer, size)) {
		chip_deselect();
		return 1;
	}
	chip_deselect();
	return 0;
}


int w25n01kv_write(const uint32_t page, const uint16_t col, const void *buffer, const size_t size)
{
	if (!w25n01kv_write_enable()) {
		chip_deselect();
		return 1;
	}
	chip_select();

	uint8_t tx1[] = {
		W25N_CMD_LOAD_PROGRAM_DATA,
		(col & 0xFF00) >> 8,
		(col & 0x00FF)
	};
	chip_select();
	if (!spi_transmit(tx1, sizeof(tx1))) {
		chip_deselect();
		return 1;
	}
	if (!spi_transmit(buffer, size)) {
		chip_deselect();
		return 1;
	};
	chip_deselect();

	uint8_t tx2[] = {
		W25N_CMD_PROGRAM_EXECUTE,
		(page & 0xFF0000) >> 16,
		(page & 0x00FF00) >> 8,
		(page & 0x0000FF)
	};
	chip_select();
	if (!spi_transmit(tx2, sizeof(tx2))) {
		chip_deselect();
		return 1;
	}
	chip_deselect();
	
	HAL_Delay(3);
	return 0;
}

int w25n01kv_erase(const uint32_t page)
{
	if (!w25n01kv_write_enable()) {
		chip_deselect();
		return 1;
	}

	uint8_t tx[] = {
		W25N_CMD_BLOCK_ERASE,
		(page & 0xFF0000) >> 16,
		(page & 0x00FF00) >> 8,
		(page & 0x0000FF)
	};
	chip_select();
	if (!spi_transmit(tx, sizeof(tx))) {
		chip_deselect();
		return 1;
	}
	chip_deselect();

	HAL_Delay(10);
	return 0;
}
