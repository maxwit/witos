#include <stdio.h>

// read CPU, SoC & Board IDs
// fixme: add ARM Cortex+ support

// ID list:
// PXA168: 0x4115926*?
//

unsigned long read_cpu_id(void)
{
	// int i;
	// char buff[4];
	// const char *arch_name[] = {"4", "4T","5", "5T", "5TE", "5TEJ", "6"};
	// const char *fmt = "ARCH = ARMv%s\n";
	unsigned long orig_id;

	asm volatile ("mrc  p15, 0, %0, c0, c0, 0"
					: "=r"(orig_id)
					:);

	printf("ARM CPU = 0x%x\n", orig_id);

#if 0
	arch_ver = (orig_id >> 16) & 0xf;

	if (arch_ver == 0xf) {
		// TODO: add code here
		printf(fmt, "6+");
	} else if (arch_ver < ARRAY_ELEM_NUM(arch_name))
		printf(fmt, arch_name[arch_ver - 1]);
	else
		return -1;

	arch_ver = (orig_id >> 4) & 0xfff;
	for (i = 8; i >= 0; i -= 4) {
		int val = (arch_ver >> i) & 0xf;
		switch (val) {
		case 0 ... 9:
			*buff = val + '0';
			break;
		case 0xa ... 0xf:
			*buff = '1';
			buff++;
			*buff = val - 10 + '0';
			break;
		}

		buff++;
	}

	*buff = '\0';

	printf("CPU  = ARM%s\n", buff);

	printf("PLAT = " CONFIG_PLAT_NAME "\n");
#endif

	return orig_id;
}

