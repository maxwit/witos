#pragma once

#include <sys/types.h>

#define container_of(ptr, type, member) \
	(type *)((char *)ptr - (long)(&((type *)0)->member))

#ifdef CONFIG_DEBUG
#define DPRINT(fmt, args ...) \
	printf(fmt, ##args)
#define GEN_DBG(fmt, args ...) \
	printf("%s() line %d: " fmt, __func__, __LINE__, ##args)
#else
#define DPRINT(fmt, args ...)
#define GEN_DBG(fmt, args ...)
#endif

// list
#define HEAD_INIT(list) \
	{.next = &list, .prev = &list}

#define DECL_INIT_LIST(list) \
	struct list_node list = HEAD_INIT(list)

struct list_node {
	struct list_node *next, *prev;
};

static inline void list_head_init(struct list_node *head)
{
	head->next = head;
	head->prev = head;
}

static inline void __list_add(struct list_node *new_node,
					struct list_node *prev, struct list_node *next)
{
	next->prev = new_node;
	new_node->next  = next;
	new_node->prev  = prev;
	prev->next = new_node;
}

static inline void list_add_head(struct list_node *new_node, struct list_node *head)
{
	__list_add(new_node, head, head->next);
}

static inline void list_add_tail(struct list_node *new_node, struct list_node *head)
{
	__list_add(new_node, head->prev, head);
}

static inline void __list_del(struct list_node *prev, struct list_node *next)
{
	next->prev = prev;
	prev->next = next;
}

static inline void list_del_node(struct list_node *node)
{
	__list_del(node->prev, node->next);
}

static inline int list_is_empty(const struct list_node *head)
{
	return head->next == head;
}

#define list_for_each(iter, head) \
	for (iter = (head)->next; iter != (head); iter = iter->next)


// file operations
int __open(const char *name, int flags, ...);

int __close(int fd);

ssize_t __read(int fd, void *buff, size_t count);

ssize_t __write(int fd, const void *buff, size_t count);

int __ioctl(int fd, int cmd, ...);

loff_t __lseek(int fd, loff_t offset, int whence);

// fixme
int __mount(const char *type, unsigned long flags, const char *bdev_name, const char *path);

int __umount(const char *mnt);
