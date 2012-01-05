#pragma once

// fixme
#ifndef ffs

static int inline generic_ffs(int x)
{
	int r = 1;

	if (!x)
		return 0;

	if (!(x & 0xffff)) {
		x >>= 16;
		r += 16;
	}
	if (!(x & 0xff)) {
		x >>= 8;
		r += 8;
	}
	if (!(x & 0xf)) {
		x >>= 4;
		r += 4;
	}
	if (!(x & 3)) {
		x >>= 2;
		r += 2;
	}
	if (!(x & 1)) {
		x >>= 1;
		r += 1;
	}

	return r;
}

#define ffs generic_ffs

#endif

