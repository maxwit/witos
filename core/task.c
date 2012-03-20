#include <errno.h>
#include <task.h>

static struct task *g_current;

struct task *get_current_task(void)
{
	return g_current;
}

void set_current_task(struct task *current)
{
	g_current = current;
}
