#ifndef W25N01KV_H
#define W25N01KV_H

#include <stddef.h>
#include <stdint.h>

#define W25N_CMD_READ_JEDEC_ID          0x9F
#define W25N_CMD_READ_STATUS_REGISTER   0x0F // Checks if Page Data Read is done (BUSY = 1) (not sure if needed for now)
#define W25N_CMD_PAGE_DATA_READ         0x13 // Reads data from a flash page into the data buffer | Resets WEL bit
#define W25N_CMD_READ_FROM_BUFFER       0x03 // Reads from the data buffer
#define W25N_CMD_WRITE_ENABLE           0x06 // Required for Load Program Data | Sets WEL bit to 1
#define W25N_CMD_WRITE_DISABLE          0x04 // Resets WEL bit to 0
#define W25N_CMD_LOAD_PROGRAM_DATA      0x02 // Loads data into the data buffer (Max 1 page)
#define W25N_CMD_PROGRAM_EXECUTE        0x10 // Programs the data in the data buffer to the specified page in flash (Also resets WEL bit to 0 when finished)
#define W25N_CMD_BLOCK_ERASE            0xD8

// General Specs (Page 5 of Datasheet)
#define W25N_PAGE_SIZE                  2048
#define W25N_PAGES_PER_BLOCK            64
#define W25N_BLOCK_SIZE                 (W25N_PAGE_SIZE * W25N_PAGES_PER_BLOCK)
#define W25N_BLOCK_COUNT                1024

int w25n01kv_read(const uint32_t page, const uint16_t col, void *buffer, const size_t size);
int w25n01kv_write(const uint32_t page, const uint16_t col, const void *buffer, const size_t size);
int w25n01kv_erase(const uint32_t page);

#endif
