#pragma once

#include <types.h>

#define BARLEN 50

#define CountVal ((nLen - pBar->nMin + 1) * 100 / (pBar->nMax - pBar->nMin + 1))
#define CountBarOff ((nLen - pBar->nMin + 1) * BARLEN / (pBar->nMax - pBar->nMin + 1))

struct process_bar
{
	u32 nMin;
	u32 nMax;
	u32 nCurr;

	char cBar[];
};

int create_progress_bar(struct process_bar **ppNew, u32 nMin, u32 nMax);

void progress_bar_set_val(struct process_bar *pBar, u32 nLen);

int delete_progress_bar(struct process_bar *pBar);
