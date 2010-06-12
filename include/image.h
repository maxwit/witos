#pragma once

#include <g-bios.h>
#include <flash/flash.h>


#define YAFFS_OOB_SIZE		16
#define YAFFS2_OOB_SIZE		64


BOOL check_part_image(PART_TYPE type, const u8 *buff);

