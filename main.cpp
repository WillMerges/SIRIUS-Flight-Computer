/*
Will Merges
*/
#define DEBUG

#include "Arduino.h"
#include "Adafruit_MPL3115A2.h"
#include "rf_protocol/rf_packet.h"
#include "charge.h"
#include "Adafruit_GPS.h"

// constants
#define XBEE_BAUD 9600
#define xbee Serial4
#define GPS_BAUD 9600
#define GPS_SERIAL Serial1

// determine whether to use watchdog
#ifdef INTERVAL_INTERRUPT
#define DELTA_TIME 10^6 //in microseconds (1 second = 10^6 microseconds)
#define SEND_PERIOD 10^6 / 5 //every period (microseconds) to send data
#else
uint64_t temptime = 0;
uint64_t temptime2 = 0;
#define DELTA_TIME 1000 //in milliseconds
#define SEND_PERIOD 200 //in milliseconds
#endif

// create static memory variables
rf_data packet;
extern size_t packet_size;
_Bool reduced = false; //guard for interrupts
Adafruit_MPL3115A2 alt2;
uint32_t uptime = 0; //number of time deltas that have passed
uint32_t time_of_accel = 0; //number of delta ts
IntervalTimer uptimer;
IntervalTimer sendtimer;
Charge charges[4] {1,2,3,4};
Adafruit_GPS gps(&GPS_SERIAL);

// no need to check for overflow, the program will not be running for 136 years
// if it is, i will likely be dead and not care
void update_clock() {
    add_uptime(packet, uptime);
    uptime++;
    if(time_of_accel != 0) {
        add_time_since_accel(packet, uptime - time_of_accel);
    }
#ifndef INTERVAL_INTERRUPT
    temptime = millis();
#endif
}

//TODO maybe implement a sample rate (in main loop, interrupts probably too slow)
// sample devices for data
void sample() {
    reduced = false;

    // read altimeter 2
    if(alt2.getAltitude(&packet->data.alt) == READ_SUCCESS) {
        set_alt_change(packet);
    }

    // read GPS
    if(gps.newNMEAreceived()) {
        if(gps.parse(gps.lastNMEA())) {
            add_lat(packet, gps.latitude);
            add_long(packet, gps.longitude);
            add_alt_gps(packet, gps.altitude); // meters above mean sea level
        }
    }

    // update charges
    for(int i=0; i < 4; i++) {
        set_charge(packet, i, charges[i].deployed);
        set_continuity(packet, i, charges[i].check_continuity());
    }
}

// send data over radio
void send() {
    reduce_packet(packet);
    reduced = true;
    xbee.write(packet->serialized, packet_size);
#ifndef INTERVAL_INTERRUPT
    temptime2 = millis();
#endif
}

void gps_init() {
    gps.begin(GPS_BAUD);
    gps.sendCommand(PMTK_SET_NMEA_UPDATE_5HZ);
    gps.sendCommand(PMTK_API_SET_FIX_CTL_5HZ);
    gps.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
    gps.sendCommand(PGCMD_ANTENNA);
}

void init() {
    xbee.begin(XBEE_BAUD);
    gps_init();
    alt2.begin();
    packet = create_packet();
#ifdef INTERVAL_INTERRUPT
    uptimer.begin(&update_clock, DELTA_TIME);
    sendtimer.begin(&send, SEND_PERIOD);
#else
    temptime = millis();
    temptime2 = temptime;
#endif
}

int main(int argc, char** argv) {
    init();
    while(1) {
        sample();
#ifndef INTERVAL_INTERRUPT
        if(millis() - temptime >= DELTA_TIME) {
            update_clock();
        }
        if(millis() - temptime2 >= SEND_PERIOD) {
            send();
        }
#endif
        yield();
    }
}
