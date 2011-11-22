#pragma once

struct list_node {
	struct list_node *next, *prev;
};

static inline void list_head_init(struct list_node *head)
{
	head->next = head;
	head->prev = head;
}

static inline void __list_add(struct list_node *new_node,
			struct list_node *prev,
			struct list_node *next)
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

