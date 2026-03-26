#include "flash.h"

#include "w25n01kv.h"
#include "gd5f1gq5xe.h"
#include "lfs.h"

#include <stdint.h>

static int flash_sync(const struct lfs_config *c)
{
	return 0;
}

// Wrappers for w25n01kv
static int w25n01kv_lfs_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t offset,
		      void *location, lfs_size_t size)
{
	uint32_t page = block * W25N_PAGES_PER_BLOCK + (offset / W25N_PAGE_SIZE);
	uint16_t col = offset % W25N_PAGE_SIZE;
	return w25n01kv_read(page, col, location, size);
}

static int w25n01kv_lfs_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t offset,
		      const void *data, lfs_size_t size)
{
	uint32_t page = block * W25N_PAGES_PER_BLOCK + (offset / W25N_PAGE_SIZE);
	uint16_t col = offset % W25N_PAGE_SIZE;
	return w25n01kv_write(page, col, data, size);
}

static int w25n01kv_lfs_erase(const struct lfs_config *c, lfs_block_t block) {
	uint32_t page = block * W25N_PAGES_PER_BLOCK;
	return w25n01kv_erase(page);
}

// Wrappers for gd5f1gq5xe
static int gd5f1gq5xe_lfs_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t offset,
		      void *location, lfs_size_t size)
{
	uint32_t page = block * GD5F_PAGES_PER_BLOCK + (offset / GD5F_PAGE_SIZE);
	uint16_t col = offset % GD5F_PAGE_SIZE;
	return gd5f1gq5xe_read(page, col, location, size);
}

static int gd5f1gq5xe_lfs_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t offset,
		      const void *data, lfs_size_t size)
{
	uint32_t page = block * GD5F_PAGES_PER_BLOCK + (offset / GD5F_PAGE_SIZE);
	uint16_t col = offset % GD5F_PAGE_SIZE;
	return gd5f1gq5xe_write(page, col, data, size);
}

static int gd5f1gq5xe_lfs_erase(const struct lfs_config *c, lfs_block_t block) {
	uint32_t page = block * GD5F_PAGES_PER_BLOCK;
	return gd5f1gq5xe_erase(page);
}

void flash_init(struct flash_dev *dev, enum flash_name name)
{
	dev->name = name;
	/* dev->config = (struct lfs_config) { */
	/* 	/\* .read  = w25n01kv_lfs_read, *\/ */
	/* 	/\* .prog  = w25n01kv_lfs_prog, *\/ */
	/* 	/\* .erase = w25n01kv_lfs_erase, *\/ */
	/* 	.read  = &gd5f1gq5xe_lfs_read, */
	/* 	.prog  = &gd5f1gq5xe_lfs_prog, */
	/* 	.erase = &gd5f1gq5xe_lfs_erase, */
	/* 	.sync  = flash_sync, */

	/* 	.read_size      = GD5F_PAGE_SIZE, */
	/* 	.prog_size      = GD5F_PAGE_SIZE, */
	/* 	.block_size     = GD5F_BLOCK_SIZE, */
	/* 	.block_count    = GD5F_BLOCK_COUNT, */
	/* 	.cache_size     = GD5F_PAGE_SIZE, */
	/* 	.lookahead_size = 128, */
	/* 	.block_cycles   = 512, */
	/* }; */
}

int flash_mount(struct flash_dev *flash, const struct lfs_config *config)
{
	gd5f1gq5xe_unlock();
	int err = lfs_mount(&(flash->lfs), config);
	if (err) {
		lfs_format(&(flash->lfs), config);
		lfs_mount(&(flash->lfs), config);
		return flash_boot_count(flash, true);
	}
	flash_boot_count(flash, true);
	return err;
}

int flash_unmount(struct flash_dev *flash)
{
	return lfs_unmount(&(flash->lfs));
}

uint32_t flash_boot_count(struct flash_dev *flash, bool update)
{
	lfs_file_t file;
	uint32_t boot_count = 0;

	lfs_file_open(&(flash->lfs), &file, "boot_count", LFS_O_RDWR | LFS_O_CREAT);
	lfs_file_read(&(flash->lfs), &file, &boot_count, sizeof(boot_count));

	if (update) {
		++boot_count;
		lfs_file_rewind(&(flash->lfs), &file);
		lfs_file_write(&(flash->lfs), &file, &boot_count, sizeof(boot_count));
	}

	lfs_file_close(&(flash->lfs), &file);
	return boot_count;
}
