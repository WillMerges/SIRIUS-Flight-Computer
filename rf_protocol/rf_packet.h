#ifndef RF_PACKET_H
#define RF_PACKET_H
#include <sys/types.h>
#include <stdint.h>

// rightward carrot!
#define START_BYTE 0x3E

#ifndef RF_DATA_PACKET
union rf_data_u {};
#endif

typedef union rf_data_u* rf_data;

// general functions
rf_data create_packet();
rf_data destroy_packet(rf_data);
rf_data clear_packet(rf_data);

// functions that add data to packet
rf_data add_alt(float alt);
rf_data add_lat(float lat);
rf_data add_long(float lon);
rf_data add_alt_gps(float alt);
rf_data add_200g_accel(int x, int y, int z);
rf_data add_16g_accel(float x, float y, float z);
rf_data add_16_mag(float x, float y, float z);
rf_data add_pitch(float pitch);
rf_data add_roll(float roll);
rf_data add_uptime(int seconds);
rf_data add_time_since_accel(int seconds);
rf_data add_temp1(int temp);
rf_data add_temp2(int temp);
rf_data set_charge1(_Bool active);
rf_data set_charge2(_Bool active);
rf_data set_charge3(_Bool active);
rf_data set_charge4(_Bool active);

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
