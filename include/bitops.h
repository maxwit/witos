#pragma once

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
