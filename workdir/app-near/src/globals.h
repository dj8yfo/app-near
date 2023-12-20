#pragma once 
#include "os.h"
#include "ux.h"
#include "os_io_seproxyhal.h"


#define P1_CONFIRM 0x01
#define P1_NON_CONFIRM 0x00

#define FULL_ADDRESS_LENGTH 60
#define BIP32_PATH 5

// display stepped screens
extern unsigned int ux_step;
extern unsigned int ux_step_count;

enum BlindSign {
    BlindSignDisabled = 0,
    BlindSignEnabled = 1,
};

typedef struct internalStorage_t
{
    uint8_t blind_sign_enabled;
    uint8_t initialized;
} internalStorage_t;

extern const internalStorage_t N_storage_real;
#define N_storage (*(volatile internalStorage_t*) PIC(&N_storage_real))
