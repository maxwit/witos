#pragma once

#include <types.h>

#define BARLEN 50

#define CountVal ((nLen - pBar->nMin + 1) * 100 / (pBar->nMax - pBar->nMin + 1))
#define CountBarOff ((nLen - pBar->nMin + 1) * BARLEN / (pBar->nMax - pBar->nMin + 1))

struct process_bar {
	__u32 nMin;
	__u32 nMax;
	__u32 nCurr;

	char cBar[];
};

int create_progress_bar(struct process_bar **ppNew, __u32 nMin, __u32 nMax);

void progress_bar_set_val(struct process_bar *pBar, __u32 nLen);

int delete_progress_bar(struct process_bar *pBar);
