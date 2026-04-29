#include "main.h"
#include "lfs.h"
#include "lfs_port.h"

extern SPI_HandleTypeDef hspi3;
static lfs_t lfs;

// ── SPI helpers ──────────────────────────────────────────────────────────────

static void csLow(void) {
    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET);
}

static void csHigh(void) {
    HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET);
}

static void spiSend(const uint8_t *data, uint16_t size) {
    HAL_SPI_Transmit(&hspi3, (uint8_t *)data, size, HAL_MAX_DELAY);
}

static void spiReceive(uint8_t *buf, uint16_t size) {
    HAL_SPI_Receive(&hspi3, buf, size, HAL_MAX_DELAY);
}

// ── LFS config ───────────────────────────────────────────────────────────────

struct lfs_config lfsconfig = {
    .read  = lfs_read,
    .prog  = lfs_prog,
    .erase = lfs_erase,
    .sync  = lfs_sync,

    .read_size      = GD5F_PAGE_SIZE,
    .prog_size      = GD5F_PAGE_SIZE,
    .block_size     = GD5F_BLOCK_SIZE,
    .block_count    = GD5F_BLOCK_COUNT,
    .cache_size     = GD5F_PAGE_SIZE,
    .lookahead_size = 128,
    .block_cycles   = 512,
};

// ── LFS callbacks ─────────────────────────────────────────────────────────────

int lfs_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t offset,
             void *buffer, lfs_size_t size)
{
    uint32_t page = block * GD5F_PAGES_PER_BLOCK + (offset / GD5F_PAGE_SIZE);
    uint16_t col  = offset % GD5F_PAGE_SIZE;

    // Load page into chip cache
    csLow();
    uint8_t cmd1[] = {
        GD5F_CMD_READ_TO_CACHE,
        0x00,
        (page >> 8) & 0xFF,
        page & 0xFF
    };
    spiSend(cmd1, 4);
    csHigh();
    HAL_Delay(1);   // tRD max ~100 µs; 1 ms is safe

    // Read from cache at column offset
    csLow();
    uint8_t cmd2[] = {
        GD5F_CMD_READ_FROM_CACHE,
        (col >> 8) & 0x0F,  // 12-bit column address (upper nibble only)
        col & 0xFF,
        0x00                 // dummy byte required by datasheet
    };
    spiSend(cmd2, 4);
    spiReceive(buffer, size);
    csHigh();

    return 0;
}

int lfs_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t offset,
             const void *data, lfs_size_t size)
{
    uint32_t page = block * GD5F_PAGES_PER_BLOCK + (offset / GD5F_PAGE_SIZE);
    uint16_t col  = offset % GD5F_PAGE_SIZE;

    // Write enable
    uint8_t we = GD5F_CMD_WRITE_ENABLE;
    csLow();
    spiSend(&we, 1);
    csHigh();

    // Load data into chip buffer
    csLow();
    uint8_t cmd1[] = {
        GD5F_CMD_PROGRAM_LOAD,
        (col >> 8) & 0x0F,
        col & 0xFF
    };
    spiSend(cmd1, 3);
    spiSend((const uint8_t *)data, size);
    csHigh();

    // Execute program (write buffer → flash page)
    csLow();
    uint8_t cmd2[] = {
        GD5F_CMD_PROGRAM_EXECUTE,
        0x00,
        (page >> 8) & 0xFF,
        page & 0xFF
    };
    spiSend(cmd2, 4);
    csHigh();
    HAL_Delay(5);   // tPROG max ~700 µs; 5 ms is safe

    return 0;
}

int lfs_erase(const struct lfs_config *c, lfs_block_t block)
{
    uint32_t page = block * GD5F_PAGES_PER_BLOCK;

    // Write enable
    uint8_t we = GD5F_CMD_WRITE_ENABLE;
    csLow();
    spiSend(&we, 1);
    csHigh();

    // Block erase
    csLow();
    uint8_t cmd[] = {
        GD5F_CMD_BLOCK_ERASE,
        0x00,
        (page >> 8) & 0xFF,
        page & 0xFF
    };
    spiSend(cmd, 4);
    csHigh();
    HAL_Delay(25);  // tBERS max ~10 ms; 25 ms is safe

    return 0;
}

int lfs_sync(const struct lfs_config *c)
{
    // Writes and erases are committed immediately; nothing to flush.
    return 0;
}

// ── Public API ────────────────────────────────────────────────────────────────

// GD5F ships with all blocks write-protected. Clear the block-lock register
// before the first mount so writes and erases succeed.
static void gd5f_unlock(void)
{
    uint8_t we = GD5F_CMD_WRITE_ENABLE;
    csLow();
    spiSend(&we, 1);
    csHigh();

    uint8_t cmd[] = { GD5F_CMD_SET_FEATURE, 0xA0, 0x00 };
    csLow();
    spiSend(cmd, 3);
    csHigh();
    HAL_Delay(25);
}

int stmlfs_mount(void)
{
    gd5f_unlock();

    int err = lfs_mount(&lfs, &lfsconfig);
    if (err) {
        lfs_format(&lfs, &lfsconfig);
        err = lfs_mount(&lfs, &lfsconfig);
    }
    return err;
}

int stmlfs_unmount(void)
{
    return lfs_unmount(&lfs);
}

int stmlfs_file_open(lfs_file_t *file, const char *path, int flags)
{
    return lfs_file_open(&lfs, file, path, flags);
}

int stmlfs_file_read(lfs_file_t *file, void *buffer, lfs_size_t size)
{
    return lfs_file_read(&lfs, file, buffer, size);
}

int stmlfs_file_rewind(lfs_file_t *file)
{
    return lfs_file_rewind(&lfs, file);
}

lfs_ssize_t stmlfs_file_write(lfs_file_t *file, const void *buffer, lfs_size_t size)
{
    return lfs_file_write(&lfs, file, buffer, size);
}

int stmlfs_file_close(lfs_file_t *file)
{
    return lfs_file_close(&lfs, file);
}
