#ifndef GD5F1GQ5XE_H
#define GD5F1GQ5XE_H

#include <stddef.h>
#include <stdint.h>

#define GD5F_SET_FEATURE      0x1F
#define GD5F_WRITE_ENABLE     0x06
#define GD5F_WRITE_DISABLE    0x04
#define GD5F_READ_TO_CACHE    0x13
#define GD5F_READ_FROM_CACHE  0x03
#define GD5F_PROGRAM_LOAD     0x02
#define GD5F_PROGRAM_EXECUTE  0x10
#define GD5F_ERASE            0xD8

#define GD5F_BLOCK_COUNT      1024
#define GD5F_PAGES_PER_BLOCK  64
#define GD5F_PAGE_SIZE        2048
#define GD5F_BLOCK_SIZE       GD5F_PAGES_PER_BLOCK * GD5F_PAGE_SIZE


int gd5f1gq5xe_read(const uint32_t page, const uint16_t col, void *buffer, const size_t size);
int gd5f1gq5xe_write(const uint32_t page, const uint16_t col, const void *buffer, const size_t size);
int gd5f1gq5xe_erase(const uint32_t page);
int gd5f1gq5xe_unlock();

#endif
