#include <g-bios.h>


#define LIST_NODE_SIZE             WORD_ALIGN_UP(sizeof(struct list_node))
#define LIST_NODE_ALIGN(size)     (((size) + LIST_NODE_SIZE - 1) & ~(LIST_NODE_SIZE - 1))
#define MIN_HEAP_LEN               1024
#define IS_FREE(size)             (((size) & (WORD_SIZE - 1)) == 0)
#define GET_SIZE(region)          ((region)->nCurrSize & ~(WORD_SIZE - 1))


struct MemRegion
{
	u32   nPrevSize;
	u32   nCurrSize;
	struct list_node lnMemRegion;
};


static struct list_node g_free_region_list;


static __INLINE__ struct MemRegion *get_successor(struct MemRegion *region)
{
	return (struct MemRegion *)((u8 *)region + DWORD_SIZE + GET_SIZE(region));
}


static __INLINE__ struct MemRegion *get_predeccessor(struct MemRegion *region)
{
	return (struct MemRegion *)((u8 *)region - (region->nPrevSize & ~(WORD_SIZE - 1)) - DWORD_SIZE);
}


static void __INLINE__ region_set_size(struct MemRegion *region, u32 size)
{
	struct MemRegion *succ_region;

	region->nCurrSize = size;

	succ_region = get_successor(region);
	succ_region->nPrevSize = size;
}


int GkInitHeap(u32 start, u32 end)
{
	struct MemRegion *first, *tail;

	start = WORD_ALIGN_UP(start);
	end   = WORD_ALIGN_DOWN(end);

	if (start + MIN_HEAP_LEN >= end)
		return -EINVAL;

	first = (struct MemRegion *)start;
	tail  = (struct MemRegion *)(end - DWORD_SIZE);  // sizeof(*tail)

	first->nPrevSize = 1;
	first->nCurrSize = (u32)tail - (u32)first - DWORD_SIZE;

	tail->nPrevSize = first->nCurrSize;
	tail->nCurrSize = 1;

	list_head_init(&g_free_region_list);

	list_add_tail(&first->lnMemRegion, &g_free_region_list);

	return 0;
}


void *malloc(u32 size)
{
	void *p = NULL;
	struct list_node *iter;
	u32 alloc_size, reset_size;
	struct MemRegion *curr_region, *succ_region;
	u32 psr;

	lock_irq_psr(psr);

	alloc_size = LIST_NODE_ALIGN(size);
	list_for_each(iter, &g_free_region_list)
	{
		curr_region = OFF2BASE(iter, struct MemRegion, lnMemRegion);
		// printf("%d <--> %d\n", curr_region->nCurrSize, alloc_size);
		if (curr_region->nCurrSize >= alloc_size)
			goto do_alloc;
	}

	unlock_irq_psr(psr);

	return NULL;

do_alloc:
	list_del_node(iter);

	reset_size = curr_region->nCurrSize - alloc_size;

	if (reset_size < sizeof(struct MemRegion))
	{
		region_set_size(curr_region, curr_region->nCurrSize | 1);
	}
	else
	{
		region_set_size(curr_region, alloc_size | 1);

		succ_region = get_successor(curr_region);
		region_set_size(succ_region, reset_size - DWORD_SIZE);
		list_add_tail(&succ_region->lnMemRegion, &g_free_region_list);
	}

	p = &curr_region->lnMemRegion;

	unlock_irq_psr(psr);

	return p;
}


void free(void *p)
{
	struct MemRegion *curr_region, *succ_region;
	u32 psr;

	lock_irq_psr(psr);

	curr_region = (struct MemRegion *)((u32)p - DWORD_SIZE);
	succ_region = get_successor(curr_region);

	if (IS_FREE(succ_region->nCurrSize))
	{
		region_set_size(curr_region, GET_SIZE(curr_region) + succ_region->nCurrSize + DWORD_SIZE);
		list_del_node(&succ_region->lnMemRegion);
	}
	else
	{
		region_set_size(curr_region, GET_SIZE(curr_region));
	}

	if (IS_FREE(curr_region->nPrevSize))
	{
		struct MemRegion *prev_region;

		prev_region = get_predeccessor(curr_region);
		region_set_size(prev_region, prev_region->nCurrSize + curr_region->nCurrSize + DWORD_SIZE);
	}
	else
	{
		list_add_tail(&curr_region->lnMemRegion, &g_free_region_list);
	}

	unlock_irq_psr(psr);
}



void *zalloc(u32 len)
{
	void *p;

	p = malloc(len);

	if (p)
		memset(p, 0, len);

	return p;
}

void *dma_malloc(size_t len, u32 *pa)
{
	void *va;

	va = malloc(len);

	*pa = (u32)va;

	return va;
}

