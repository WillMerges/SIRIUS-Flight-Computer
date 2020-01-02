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
    uint32_t update_mask : 14;
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
update_bit_pos update_pos;
void add_alt(rf_data packet, float alt) {
    packet->data.alt = alt;
    update_pos = ALT;
    packet->data.update_mask |= 2^update_pos; //bit 0
}

void add_lat(rf_data packet, float lat) {
    packet->data.lat = lat;
    update_pos = LAT;
    packet->data.update_mask |= 2^update_pos; //bit 1
}

void add_long(rf_data packet, float lon) {
    packet->data.lon = lon;
    update_pos = LONG;
    packet->data.update_mask |= 2^update_pos; //bit 2
}

void add_alt_gps(rf_data packet, float alt) {
    packet->data.alt_gps = alt;
    update_pos = ALTGPS;
    packet->data.update_mask |= 2^update_pos; //bit 3
}

void add_200g_accel(rf_data packet, int x, int y, int z) {
    packet->data.x200g = x;
    packet->data.y200g = y;
    packet->data.z200g = z;
    update_pos = A200G;
    packet->data.update_mask |= 2^update_pos; //bit 4
}

void add_16g_accel(rf_data packet, float x, float y, float z) {
    packet->data.x16g = x;
    packet->data.y16g = y;
    packet->data.z16g = z;
    update_pos = A16G;
    packet->data.update_mask |= 2^update_pos; //bit 5
}

void add_16_mag(rf_data packet, float x, float y, float z) {
    packet->data.x16mag = x;
    packet->data.y16mag = y;
    packet->data.z16mag = z;
    update_pos = MAG16G;
    packet->data.update_mask |= 2^update_pos; //bit 6
}

void add_pitch(rf_data packet, float pitch) {
    packet->data.pitch = pitch;
    update_pos = PITCH;
    packet->data.update_mask |= 2^PITCH; //bit 7
}

void add_roll(rf_data packet, float roll) {
    packet->data.roll = roll;
    update_pos = ROLL;
    packet->data.update_mask |= 2^update_pos; //bit 8
}

void add_uptime(rf_data packet, int seconds) {
    packet->data.uptime = seconds;
    update_pos = UPTIME;
    packet->data.update_mask |= 2^UPTIME; //bit 9
}

void add_time_since_accel(rf_data packet, int seconds) {
    packet->data.time_since_accel = seconds;
    update_pos = TIMEACCEL;
    packet->data.update_mask |= 2^update_pos; //bit 10
}

void add_temp1(rf_data packet, int temp) {
    packet->data.temp1 = temp;
    update_pos = TEMP1;
    packet->data.update_mask |= 2^update_pos; //bit 11
}

void add_temp2(rf_data packet, int temp) {
    packet->data.temp2 = temp;
    update_pos = TEMP2;
    packet->data.update_mask |= 2^update_pos; //bit 12
}

// charges ordered from 0-3
void set_charge(rf_data, int charge, _Bool active) {
    uint8_t mask = 1 << charge;
    packet->data.charges |= mask;
    update_pos = CHARGES;
    packet->data.update_mask |= 2^update_pos;
}

// retrieving data functions
//
//TODO
