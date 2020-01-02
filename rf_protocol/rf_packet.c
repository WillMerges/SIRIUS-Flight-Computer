#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>

#pragma pack(1)
struct rf_data_s {
    uint8_t start_byte;
    float alt;
    float lat;
    float lon;
    float alt_gps;
    int x200g;
    int y200g;
    int z200g;
    float x16g;
    float y16g;
    float z16g;
    float x16mag;
    float y16mag;
    float z16mag;
    float pitch;
    float roll;
    int uptime;
    int time_since_accel;
    int temp1;
    int temp2;
    uint8_t charges : 4;
    uint32_t update_mask : 19;
};
// each struct member is a bit in update_mask
// ex. alt is bit 0 (least significant)
// if bit is a 1, the data was updated, if it's a 0 it was not

union rf_data_u {
    struct rf_data_s data;
    char serialized[sizeof(struct rf_data_s)];
};

#define RF_DATA_PACKET
#include "rf_packet.h"

rf_data create_packet() {
        struct rf_data_s data;
        data.start_byte = START_BYTE;
        data.update_mask = 0;
        rf_data packet = malloc(sizeof(union rf_data_u));
        packet->data = data;
        return packet;
}

rf_data clear_packet(rf_data packet) {
    for(int i=0; i<sizeof(packet->data); i++) {
        packet->serialized[i] = 0;
    }
    packet->data.start_byte = START_BYTE;
}

rf_data destroy_packet(rf_data packet) {
    free(packet);
}


// currently for testing with sizeof only
int main(int argc, char argv[][argc]) {
    rf_data packet = create_packet();
    printf("%lu", sizeof(packet->data));
}
