#include <flash/flash.h>
//fixme:

static void usage(int argc, char *argv[])
{
	printf("Usage: ls [OPTION]... [FILE]...\n"
		"List information about the FILEs (the current directory by default).\n"
		"\nOPTION:\n"
		"\t-l : list detail information of the file \n"
		"\t-h : human readable\n");
}

int main(int argc, char *argv[])
{
	usage(argc, argv);

	return 0;
}
