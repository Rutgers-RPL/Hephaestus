#ifndef FLASH_H
#define FLASH_H

#include "lfs.h"

#include <stdint.h>

enum flash_name {
	W25N01KV,
	GD5F1GQ5XE
};

struct flash_dev {
	enum flash_name name;
	/* struct lfs_config config; */
	lfs_t lfs;
};

void flash_init(struct flash_dev *dev, enum flash_name name);
int flash_mount(struct flash_dev *flash, const struct lfs_config *config);
int flash_unmount(struct flash_dev *flash);
uint32_t flash_boot_count(struct flash_dev *flash, bool update);

#endif
