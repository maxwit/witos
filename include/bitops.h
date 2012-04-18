#pragma once
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

#define hweight8(w)		\
		((!!((w) & (1ULL << 0))) +	\
		 (!!((w) & (1ULL << 1))) +	\
		 (!!((w) & (1ULL << 2))) +	\
		 (!!((w) & (1ULL << 3))) +	\
		 (!!((w) & (1ULL << 4))) +	\
		 (!!((w) & (1ULL << 5))) +	\
		 (!!((w) & (1ULL << 6))) +	\
		 (!!((w) & (1ULL << 7))))

#define hweight16(w) (hweight8(w)  + hweight8((w)  >> 8 ))
#define hweight32(w) (hweight16(w) + hweight16((w) >> 16))
#define hweight64(w) (hweight32(w) + hweight32((w) >> 32))
