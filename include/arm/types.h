#pragma once

#define WORD_SIZE              sizeof(long)
#define WORD_ALIGN_UP(addr)    (((addr) + WORD_SIZE - 1) & ~(WORD_SIZE - 1))
#define WORD_ALIGN_DOWN(addr)  ((addr) & ~(WORD_SIZE - 1))
#define DWORD_SIZE             (WORD_SIZE << 1)
#define WORD_BITS              (WORD_SIZE * 8)
