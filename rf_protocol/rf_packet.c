#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>

struct rf_data_s {
    char start_byte;
    // other stuff
};

#define RF_DATA_PACKET
#include "rf_packet.h"
