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

#define RF_DATA_PACKET
#include "rf_packet.h"

// general functions
//
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

// adding data functions
//
void add_alt(rf_data packet, float alt) {
    packet->data.alt = alt;
    packet->data.update_mask |= 2^ALT_POS; //bit 0
}

void add_lat(rf_data packet, float lat) {
    packet->data.lat = lat;
    packet->data.update_mask |= 2^LAT_POS; //bit 1
}

void add_long(rf_data packet, float lon) {
    packet->data.lon = lon;
    packet->data.update_mask |= 2^LONG_POS; //bit 2
}

void add_alt_gps(rf_data packet, float alt) {
    packet->data.alt_gps = alt;
    packet->data.update_mask |= 2^ALTGPS_POS; //bit 3
}
//TODO finish these
void add_200g_accel(rf_data, int x, int y, int z);
void add_16g_accel(rf_data, float x, float y, float z);
void add_16_mag(rf_data, float x, float y, float z);
void add_pitch(rf_data, float pitch);
void add_roll(rf_data, float roll);
void add_uptime(rf_data, int seconds);
void add_time_since_accel(rf_data, int seconds);
void add_temp1(rf_data, int temp);
void add_temp2(rf_data, int temp);
void set_charge1(rf_data, _Bool active);
void set_charge2(rf_data, _Bool active);
void set_charge3(rf_data, _Bool active);
void set_charge4(rf_data, _Bool active);

// retrieving data functions
//
//TODO

// currently for testing with sizeof only
int main(int argc, char argv[][argc]) {
    rf_data packet = create_packet();
    printf("%lu", sizeof(packet->data));
}
