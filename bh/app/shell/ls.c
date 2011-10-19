#include <flash/flash.h>
//fixme:

static void ls_usage(void)
{
	printf("Usage: ls [OPTION]... [FILE]...\n"
		"List information about the FILEs (the current directory by default).\n"
		"\nOPTION:\n"
		"\t-l : list detail information of the file \n"
		"\t-h : human readable\n");
}

int main(int argc, char *argv[])
{
	ls_usage();

	return 0;
}
