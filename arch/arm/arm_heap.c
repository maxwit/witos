#include <g-bios.h>

static int __INIT__ heap_init(void)
{
	extern u32 _start[];
	u32 heap_start, heap_end;

	heap_start = (u32)_start - HEAP_SIZE;
	heap_end = (u32)_start;

	printf("heap init, region = [0x%08x, 0x%08x]\n",
			heap_start, heap_end);

	return GkInitHeap(heap_start, heap_end);
}

ARCH_INIT(heap_init);

