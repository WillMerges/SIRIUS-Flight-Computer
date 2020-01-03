#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <string.h>

#pragma pack(1)
struct rf_data_s {
    uint8_t start_byte;
    uint16_t update_mask : 14;
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
};
// each struct member has a bit in update_mask
// any data with an xyz only has one bit however
// assumed all xyz data is updated each time new data is added
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
    for(size_t i=0; i < sizeof(packet->data); i++) {
        packet->serialized[i] = 0;
    }
    packet->data.start_byte = START_BYTE;
    return packet;
}

void destroy_packet(rf_data packet) {
    free(packet);
}

// the packet will NOT be able to be accessed after reducing
// this functions should be changed if packet changes
// modifies the serialized attribute
// returns number of bytes packet is
size_t reduce_packet(rf_data packet) {
    int c = 3;
    int j = 0;
    for(size_t i=3; i < sizeof(*packet); i+=4) {;
        if(packet->data.update_mask & (1 << j)) {
            if(i == 79) {
                packet->serialized[c] = packet->serialized[i];
                c++;
            } else { //this might break if endianness switches
                memcpy(packet->serialized+c, packet->serialized+i, 4);
                c += 4;
            }
        }
        if(i == 19 || i == 31 || i == 43) {
            i += 8;
        }
        j++;
    }
#ifdef DEBUG
    printf("packet reduced to %d bytes\n", c);
#endif
    return c;
}

// "un reduce" a packet
static rf_data swap = NULL;
rf_data decompress_packet(rf_data packet) {
    if(!swap) {
        swap = calloc(sizeof(union rf_data_u), 1);
        swap->serialized[0] = START_BYTE;
    } else {
        clear_packet(swap);
    }
    swap->serialized[1] = packet->serialized[1];
    swap->serialized[2] = packet->serialized[2];
    int c = 3;
    int j = 0;
    for(size_t i=3; i<sizeof(union rf_data_u); i+=4) {
        if(packet->data.update_mask & (1 << j)) {
            if(i == 79) {
                swap->serialized[i] = packet->serialized[c];
                c++;
            } else {
                memcpy(swap->serialized+i, packet->serialized+c, 4);
                c+=4;
            }
        }
        if(i == 19 || i == 31 || i == 43) {
            i += 8;
        }
        j++;
    }
    void* temp = swap;
    swap = packet;
    return temp;
}

void cleanup_packet_lib() {
    if(swap) {
        free(swap);
    }
}

_Bool isLittleEndian(rf_data packet) {
    return packet->data.start_byte == '<';
}

_Bool isBigEndian(rf_data packet) {
    return packet->data.start_byte == '>';
}

// adding data functions
//
update_bit_pos update_pos;
void add_alt(rf_data packet, float alt) {
    packet->data.alt = alt;
    update_pos = ALT;
    packet->data.update_mask |= 1<<update_pos; //bit 0
}

void add_lat(rf_data packet, float lat) {
    packet->data.lat = lat;
    update_pos = LAT;
    packet->data.update_mask |= 1<<update_pos; //bit 1
}

void add_long(rf_data packet, float lon) {
    packet->data.lon = lon;
    update_pos = LONG;
    packet->data.update_mask |= 1<<update_pos; //bit 2
}

void add_alt_gps(rf_data packet, float alt) {
    packet->data.alt_gps = alt;
    update_pos = ALTGPS;
    packet->data.update_mask |= 1<<update_pos; //bit 3
}

void add_200g_accel(rf_data packet, int x, int y, int z) {
    packet->data.x200g = x;
    packet->data.y200g = y;
    packet->data.z200g = z;
    update_pos = A200G;
    packet->data.update_mask |= 1<<update_pos; //bit 4
}

void add_16g_accel(rf_data packet, float x, float y, float z) {
    packet->data.x16g = x;
    packet->data.y16g = y;
    packet->data.z16g = z;
    update_pos = A16G;
    packet->data.update_mask |= 1<<update_pos; //bit 5
}

void add_16_mag(rf_data packet, float x, float y, float z) {
    packet->data.x16mag = x;
    packet->data.y16mag = y;
    packet->data.z16mag = z;
    update_pos = MAG16G;
    packet->data.update_mask |= 1<<update_pos; //bit 6
}

void add_pitch(rf_data packet, float pitch) {
    packet->data.pitch = pitch;
    update_pos = PITCH;
    packet->data.update_mask |= 1<<PITCH; //bit 7
}

void add_roll(rf_data packet, float roll) {
    packet->data.roll = roll;
    update_pos = ROLL;
    packet->data.update_mask |= 1<<update_pos; //bit 8
}

void add_uptime(rf_data packet, int seconds) {
    packet->data.uptime = seconds;
    update_pos = UPTIME;
    packet->data.update_mask |= 1<<UPTIME; //bit 9
}

void add_time_since_accel(rf_data packet, int seconds) {
    packet->data.time_since_accel = seconds;
    update_pos = TIMEACCEL;
    packet->data.update_mask |= 1<<update_pos; //bit 10
}

void add_temp1(rf_data packet, int temp) {
    packet->data.temp1 = temp;
    update_pos = TEMP1;
    packet->data.update_mask |= 1<<update_pos; //bit 11
}

void add_temp2(rf_data packet, int temp) {
    packet->data.temp2 = temp;
    update_pos = TEMP2;
    packet->data.update_mask |= 1<<update_pos; //bit 12
}

// charges ordered from 0-3
void set_charge(rf_data packet, int charge, _Bool active) {
    uint8_t mask = active << charge;
    packet->data.charges |= mask;
    update_pos = CHARGES;
    packet->data.update_mask |= 1<<update_pos; //bit 13
}

// retrieving data functions
// could just access data directly to reduce function overhead
float get_alt(rf_data packet) {
    return packet->data.alt;
}

float get_lat(rf_data packet) {
    return packet->data.lat;
}

float get_long(rf_data packet) {
    return packet->data.lon;
}

float get_alt_gps(rf_data packet) {
    return packet->data.alt_gps;
}

int get_200g_x(rf_data packet) {
    return packet->data.x200g;
}

int get_200g_y(rf_data packet) {
    return packet->data.y200g;
}

int get_200g_z(rf_data packet) {
    return packet->data.z200g;
}

float get_16g_x(rf_data packet) {
    return packet->data.x16g;
}

float get_16g_y(rf_data packet) {
    return packet->data.y16g;
}

float get_16g_z(rf_data packet) {
    return packet->data.z16g;
}

float get_16mag_x(rf_data packet) {
    return packet->data.x16mag;
}

float get_16mag_y(rf_data packet) {
    return packet->data.y16mag;
}

float get_16mag_z(rf_data packet) {
    return packet->data.z16mag;
}

float get_pitch(rf_data packet) {
    return packet->data.pitch;
}

float get_roll(rf_data packet) {
    return packet->data.roll;
}

int get_uptime(rf_data packet) {
    return packet->data.uptime;
}

int get_time_since_accel(rf_data packet) {
    return packet->data.time_since_accel;
}

int get_temp1(rf_data packet) {
    return packet->data.temp1;
}

int get_temp2(rf_data packet) {
    return packet->data.temp2;
}

_Bool get_charge1(rf_data packet) {
    return packet->data.charges & 1; //0b1
}

_Bool get_charge2(rf_data packet) {
    return packet->data.charges & 2; //0b10
}

_Bool get_charge3(rf_data packet) {
    return packet->data.charges & 4; //0b100
}

_Bool get_charge4(rf_data packet) {
    return packet->data.charges & 8; //0b1000
}

#ifdef DEBUG
// function will change based on format of struct
// most data types reversed if on little endian system
// the update mask if set to print correctly in little endian systems
void print_packet(const rf_data packet) {
    char* s = (char*) packet;
    printf("start: %c\n", (unsigned int) *s);
    printf("update: %02x%02x\n", *(s+2), *(s+1));
    int i = 3;
    while(i < 80) {
        if((i-3)%4 == 0) {
            if(i < 10) { //add extra space
                printf("%d:  ", i);
            } else {
                printf("%d: ", i);
            }
        }
        printf("%02x ", (unsigned int) *(s+i));
        i++;
        if((i+1)%4 == 0) {
            printf("\n");
        }
    }
    printf("\n\n");
}

int main() {
    rf_data packet = create_packet();
    clear_packet(packet);
    add_temp1(packet, 0x11111111);
    add_alt(packet, 0x11111111);

    printf("%04x\n", packet->data.update_mask);

    print_packet(packet);

    reduce_packet(packet);
    printf("\n");
    print_packet(packet);

    rf_data ret = decompress_packet(packet);
    print_packet(ret);

    destroy_packet(ret);
    cleanup_packet_lib();
}

#endif
