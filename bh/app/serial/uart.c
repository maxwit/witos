#include <loader.h>
#include <uart/ymodem.h>
#include <uart/kermit.h>
#include <sysconf.h>
#include <getopt.h>

static void usage(int argc, char *argv[])
{
#if 0

Usage: uart <command> [options]
Load file from uart with kermit, and write to storage or memory only(default to storage).

command list:
	load
	send
	setup

See 'disk <command> -h' for more information on a specific command.

generic options:
	-p [k/kermit/y/ymodem]
                specify which protocol to use (kermit or ymodem)
	-m [ADDR]   load data to memory, but not write to storage.
                if ADDR is not specified, the malloc one
	-h          this help.

#endif
}

int main(int argc, char *argv[])
{
	return 0;
}
