#ifndef INC_LFS_PORT_H_
#define INC_LFS_PORT_H_

#include "lfs.h"
#include "lfs_util.h"

// GD5F1GQ5XE command opcodes
#define GD5F_CMD_WRITE_ENABLE       0x06
#define GD5F_CMD_SET_FEATURE        0x1F
#define GD5F_CMD_READ_TO_CACHE      0x13
#define GD5F_CMD_READ_FROM_CACHE    0x03
#define GD5F_CMD_PROGRAM_LOAD       0x02
#define GD5F_CMD_PROGRAM_EXECUTE    0x10
#define GD5F_CMD_BLOCK_ERASE        0xD8

// General specs
#define GD5F_PAGE_SIZE              2048
#define GD5F_PAGES_PER_BLOCK        64
#define GD5F_BLOCK_SIZE             (GD5F_PAGE_SIZE * GD5F_PAGES_PER_BLOCK)
#define GD5F_BLOCK_COUNT            1024

int lfs_read (const struct lfs_config *c, lfs_block_t block, lfs_off_t offset, void *buffer,       lfs_size_t size);
int lfs_prog (const struct lfs_config *c, lfs_block_t block, lfs_off_t offset, const void *data,   lfs_size_t size);
int lfs_erase(const struct lfs_config *c, lfs_block_t block);
int lfs_sync (const struct lfs_config *c);

int          stmlfs_mount(void);
int          stmlfs_unmount(void);
int          stmlfs_file_open   (lfs_file_t *file, const char *path, int flags);
int          stmlfs_file_read   (lfs_file_t *file, void *buffer, lfs_size_t size);
int          stmlfs_file_rewind (lfs_file_t *file);
lfs_ssize_t  stmlfs_file_write  (lfs_file_t *file, const void *buffer, lfs_size_t size);
int          stmlfs_file_close  (lfs_file_t *file);

#endif /* INC_LFS_PORT_H_ */
