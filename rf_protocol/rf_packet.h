#ifndef RF_PACKET_H
#define RF_PACKET_H
#include <sys/types.h>
#include <stdint.h>

// rightward carrot!
#define START_BYTE 0x3E

// update field bit positions
#define ALT_POS 0
#define LAT_POS 1
#define LONG_POS 2
#define ALTGPS_POS 3
//TODO finish these

#ifndef RF_DATA_PACKET
union rf_data_u {};
#endif

typedef union rf_data_u* rf_data;

// general functions
rf_data create_packet();
rf_data destroy_packet(rf_data);
rf_data clear_packet(rf_data);

// functions that add data to packet
void add_alt(rf_data, float alt);
void add_lat(rf_data, float lat);
void add_long(rf_data, float long);
void add_alt_gps(rf_data, float alt);
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

// functions that get data from packet
float get_alt(rf_data);
float get_lat(rf_data);
float get_long(rf_data);
float get_alt_gps(rf_data);
int get_200g_x(rf_data);
int get_200g_y(rf_data);
int get_200g_z(rf_data);
float get_16g_x(rf_data);
float get_16g_y(rf_data);
float get_16g_z(rf_data);
float get_16mag_x(rf_data);
float get_16mag_y(rf_data);
float get_16mag_z(rf_data);
float get_pitch(rf_data);
float get_roll(rf_data);
int get_uptime(rf_data);
int get_time_since_accel(rf_data);
int get_temp1(rf_data);
int get_temp2(rf_data);
_Bool get_charge1(rf_data);
_Bool get_charge2(rf_data);
_Bool get_charge3(rf_data);
_Bool get_charge4(rf_data);

#endif
