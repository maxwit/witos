#pragma once
#include <types.h>

struct Logo {
  u32   width;
  u32   height;
  u32   bytes_per_pixel; /* 3:RGB, 4:RGBA */
  u8     pixel_data[480 * 272 * 3 + 1];
};
