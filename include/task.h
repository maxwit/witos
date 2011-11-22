#pragma once

#include <types.h>
#include <init.h>

#define __GSECT_EXE__       __attribute__((section(".gsect_exe")))
#define __GSECT_HELP__      __attribute__((section(".gsect_help")))

#define MAX_ARG_LEN         512
#define MAX_ARGC            64

// command
struct command {
	const char *name;
	int (*main)(int argc, char *argv[]);
};

#define REGISTER_EXECUTIVE(id, entry) \
	static const __USED__ __GSECT_EXE__ struct command __g_exe_##id##__ = { \
		.name = #id, \
		.main = entry, \
	}

// help
struct option {
	const char *opt;
	const char *desc;
};

struct help_info {
	const char *name;
	const char *desc;
	int level;
	int count;
	union {
		const struct help_info *cmdv;
		const struct option *optv;
		const void *list;
	} u;
};

#define REGISTER_HELP_INFO(n, d, l, v) \
	static const __USED__ __GSECT_HELP__ struct help_info __g_help_##n##__ = { \
		.name  = #n, \
		.desc  = d, \
		.level = l, \
		.count = ARRAY_ELEM_NUM(v), \
		.u.list = v, \
	}

#define REGISTER_HELP_L1(n, d, v) REGISTER_HELP_INFO(n, d, 1, v)
#define REGISTER_HELP_L2(n, d, v) REGISTER_HELP_INFO(n, d, 2, v)

// run-time task
struct task {
	int argc;
	char **argv;
	const struct command *exe;
	const struct help_info *help;
};

struct task *get_current_task(void);
void set_current_task(struct task *);
