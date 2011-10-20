
struct args_list {
	const char *name;
	struct options {
		const char *cmd;
		const char *opt;
		const char *desc;
	} opt[0];
};

