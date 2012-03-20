#pragma once

#if defined(CONFIG_BOARD_BEAGLE)
#define SMSC91X_BASE      GPMC_CS_BASE(1)
#elif defined(CONFIG_BOARD_SMDK6410)
#define SMSC91X_BASE      ??
#endif

// TODO:

#define SMSC91C111_ID 0x3391
