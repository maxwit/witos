#include <bar.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <malloc.h>

int create_progress_bar(struct process_bar **ppNew, __u32 nMin, __u32 nMax)
{
	struct process_bar *pTmp;

	pTmp = (struct process_bar *)malloc(sizeof(struct process_bar) + BARLEN + 3);

	if (NULL == pTmp)
		return -1;

	pTmp->nMin  = nMin;
	pTmp->nMax  = nMax;
	pTmp->nCurr = 1;

	memset(pTmp->cBar + 1, ' ', BARLEN);

	pTmp->cBar[0] = '[';
	pTmp->cBar[BARLEN + 1] = ']';
	pTmp->cBar[BARLEN + 2] = '\0';
	pTmp->cBar[BARLEN / 2 + 1] = '%';

	*ppNew = pTmp;

	return 0;
}

void progress_bar_set_val(struct process_bar *pBar, __u32 nLen)
{
	while (pBar->nCurr <= CountBarOff) {
		pBar->cBar[pBar->nCurr]          = '=';
		pBar->cBar[1 + pBar->nCurr++] = '>';
		pBar->cBar[BARLEN / 2 - 2]       = ' ';
		pBar->cBar[BARLEN / 2 + 1]       = '%';
		pBar->cBar[BARLEN / 2 + 2]       = ' ';
	}

	pBar->cBar[BARLEN / 2 -1] = CountVal / 10 + '0';
	pBar->cBar[BARLEN / 2]     = CountVal % 10 + '0';

	if (CountVal == 100) {
		pBar->cBar[BARLEN / 2 - 3] = ' ';
		pBar->cBar[BARLEN / 2 - 2] = '1';
		pBar->cBar[BARLEN / 2 - 1] = '0';
		pBar->cBar[BARLEN + 1]  = ']';
	}

	printf("\r%s", pBar->cBar);
//	fflush(1);
}

int delete_progress_bar(struct process_bar *pBar)
{
	if (NULL == pBar) {
		BUG();
		return -1;
	}

	free(pBar);

	return 0;
}
