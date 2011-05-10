#pragma once

#include <types.h>

int gk_init_heap(u32 start, u32 end);

void *malloc(u32 size);

void free(void *p);

void *zalloc(u32);

void *dma_malloc(size_t len, u32 *ulPhyAddr);

#define SAFE_FREE(p) \
	do \
	{  \
		free(p); \
		p = NULL; \
	} while (0)

