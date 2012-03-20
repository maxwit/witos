#include <list.h>
#include <init.h>
#include <errno.h>
#include <malloc.h>
#include <assert.h>
#include <string.h>

#define LIST_NODE_SIZE             WORD_ALIGN_UP(sizeof(struct list_node))
#define LIST_NODE_ALIGN(size)     (((size) + LIST_NODE_SIZE - 1) & ~(LIST_NODE_SIZE - 1))
#define MIN_HEAP_LEN               1024
#define IS_FREE(size)             (((size) & (WORD_SIZE - 1)) == 0)
#define GET_SIZE(region)          ((region)->curr_size & ~(WORD_SIZE - 1))

struct mem_region {
	size_t pre_size;
	size_t curr_size;
	struct list_node ln_mem_region;
};

static DECL_INIT_LIST(g_free_region_list);

static inline struct mem_region *get_successor(struct mem_region *region)
{
	return (struct mem_region *)((__u8 *)region + DWORD_SIZE + GET_SIZE(region));
}

static inline struct mem_region *get_predeccessor(struct mem_region *region)
{
	return (struct mem_region *)((__u8 *)region - (region->pre_size & ~(WORD_SIZE - 1)) - DWORD_SIZE);
}

static void inline region_set_size(struct mem_region *region, size_t size)
{
	struct mem_region *succ_region;

	region->curr_size = size;

	succ_region = get_successor(region);
	succ_region->pre_size = size;
}

static int __INIT__ __heap_init(unsigned long start, unsigned long end)
{
	struct mem_region *first, *tail;

	start = WORD_ALIGN_UP(start);
	end   = WORD_ALIGN_DOWN(end);

	if (start + MIN_HEAP_LEN >= end)
		return -EINVAL;

	first = (struct mem_region *)start;
	tail  = (struct mem_region *)(end - DWORD_SIZE);  // sizeof(*tail)

	first->pre_size = 1;
	first->curr_size = (unsigned long)tail - (unsigned long)first - DWORD_SIZE;

	tail->pre_size = first->curr_size;
	tail->curr_size = 1;

	list_add_tail(&first->ln_mem_region, &g_free_region_list);

	return 0;
}

int __INIT__ heap_init(void)
{
	unsigned long heap_start, heap_end;
#ifdef CONFIG_NORMAL_SPACE
	extern unsigned long _end[];

	heap_start = (unsigned long)_end;
	heap_end   = heap_start + CONFIG_HEAP_SIZE;
#else
	extern unsigned long _start[];

	heap_end   = (unsigned long)_start;
	heap_start = heap_end - CONFIG_HEAP_SIZE;
#endif

	DPRINT("%s(): region = [0x%08x, 0x%08x], size = 0x%08x\n",
			__func__, heap_start, heap_end, heap_end - heap_start);

	return __heap_init(heap_start, heap_end);
}

void *malloc(size_t size)
{
	void *p = NULL;
	struct list_node *iter;
	size_t alloc_size, reset_size;
	struct mem_region *curr_region, *succ_region;
	unsigned long __UNUSED__ psr;

	lock_irq_psr(psr);

	alloc_size = LIST_NODE_ALIGN(size);
	list_for_each(iter, &g_free_region_list) {
		curr_region = container_of(iter, struct mem_region, ln_mem_region);
		if (curr_region->curr_size >= alloc_size)
			goto do_alloc;
	}

	unlock_irq_psr(psr);

	return NULL;

do_alloc:
	list_del_node(iter);

	reset_size = curr_region->curr_size - alloc_size;

	if (reset_size < sizeof(struct mem_region))
		region_set_size(curr_region, curr_region->curr_size | 1);
	else {
		region_set_size(curr_region, alloc_size | 1);

		succ_region = get_successor(curr_region);
		region_set_size(succ_region, reset_size - DWORD_SIZE);
		list_add_tail(&succ_region->ln_mem_region, &g_free_region_list);
	}

	p = &curr_region->ln_mem_region;

	unlock_irq_psr(psr);

	return p;
}

void free(void *p)
{
	struct mem_region *curr_region, *succ_region;
	unsigned long __UNUSED__ psr;

	lock_irq_psr(psr);

	curr_region = (struct mem_region *)((unsigned long)p - DWORD_SIZE);
	succ_region = get_successor(curr_region);

	if (IS_FREE(succ_region->curr_size)) {
		region_set_size(curr_region, GET_SIZE(curr_region) + succ_region->curr_size + DWORD_SIZE);
		list_del_node(&succ_region->ln_mem_region);
	} else
		region_set_size(curr_region, GET_SIZE(curr_region));

	if (IS_FREE(curr_region->pre_size)) {
		struct mem_region *prev_region;

		prev_region = get_predeccessor(curr_region);
		region_set_size(prev_region, prev_region->curr_size + curr_region->curr_size + DWORD_SIZE);
	} else
		list_add_tail(&curr_region->ln_mem_region, &g_free_region_list);

	unlock_irq_psr(psr);
}

void *zalloc(size_t size)
{
	void *p;

	p = malloc(size);

	if (p)
		memset(p, 0, size);

	return p;
}

void *dma_alloc_coherent(size_t size, unsigned long *pa)
{
	void *va;

	va = malloc(size);
	if (NULL == va)
		return NULL;

	*pa = (unsigned long)va;

	return va;
}

struct list_node *get_heap_head_list(void)
{
	return &g_free_region_list;
}
