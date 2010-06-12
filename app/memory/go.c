#include <g-bios.h>
#include <string.h>

int main(int argc, char *argv[])
{
	void (*fn)(void);

	if (argc < 2)
		return -EINVAL;

	string2value(argv[1], (u32 *)&fn);
	printf("goto 0x%08x ...\n", (u32)fn);

	fn();

	return -EIO;
}
