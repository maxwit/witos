#include <loader.h>
#include <uart/ymodem.h>
#include <uart/kermit.h>
#include <sysconf.h>
#include <getopt.h>

static void usage(int argc, char *argv[])
{
#if 0

Usage: uart <command> [<args>]
Load file from uart with kermit, and write to storage or memory only (default to storage).

command list:r
	load
	send
	config
	test

See 'uart <command> -h' for more information on a specific command.

generic options:
	-i <num>    UART interface ID, int number (0, 1, 2, ...) or
                string name (ttyS0, ttySAC1, ...). default from sysconfig.
	-p <k|kermit|y|ymodem>
                the protocol to use (kermit or ymodem). default from sysconfig.
	-f <file>   file name.
	-m [ADDR]   load data to memory, but not write to storage.
                if ADDR is not specified, then malloc one.
	-v          verbose mode.
	-h          this help.

#endif
}

int main(int argc, char *argv[])
{
	usage(argc, argv);

	return 0;
}
