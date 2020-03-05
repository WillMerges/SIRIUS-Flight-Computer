/*
Will Merges
IREC 2020 "Blackout" OFP (Operational Flight Program)
*/

// TODO data is potentially sent while packet is updating data due to watchdog
// also, never returns anywhere from interrupt, kinda funky
// need to fix that stuff
// for now, dont use interval timer and do NOT define INTERVAL_INTERRUPT in Makefile

// TODO figure out which altimeter to use and when
// TODO add buttons for test charges mode and arming
// TODO add wireless ignition for test charge deployment mode
// TODO figure out launch detection and arming of charges (arm either in test mode or after launch)
// TODO charge deployment (Kalman filter, other methods)
// TODO figure out calibration for imu + altimeters

#define DEBUG

#include "Arduino.h"
#include "Adafruit_MPL3115A2.h"
#include "rf_protocol/rf_packet.h"
#include "charge.h"
#include "Adafruit_GPS.h"
#include "ADXL377.h"
#include "SparkFunLSM9DS1.h"
#include "MS5607.h"

// to account for uneven magnetic field, declination of location must be set
#define DECLINATION -13 // degrees declination in NY

// constants
#define XBEE_BAUD 9600
#define xbee Serial4
#define GPS_BAUD 9600
#define GPS_SERIAL Serial1
#define ACCEL200GX_PIN 37
#define ACCEL200GY_PIN 38
#define ACCEL200GZ_PIN 39
#define ALT_SWITCH_THRESHOLD 10000 // 10000 feet, switch altimeters

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
uint32_t uptime = 0; //number of time deltas that have passed
uint32_t time_of_accel = 0; //number of delta ts
IntervalTimer uptimer;
IntervalTimer sendtimer;
Charge charges[4] {1,2,3,4};
Adafruit_GPS gps(&GPS_SERIAL);
_Bool gps_online = true;
ADXL377 accel200g(ACCEL200GX_PIN, ACCEL200GY_PIN, ACCEL200GZ_PIN);
LSM9DS1 imu;
_Bool imu_online = true;
MS5607 alt1;
_Bool alt1_online = true;
Adafruit_MPL3115A2 alt2;
_Bool alt2_online = true;
_Bool high_alt = false;

// arming states
enum arm_states {
    ARMED, ARMED_NO_CHARGE, DISARMED
};
enum arm_states arm_state = DISARMED;

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

// TODO maybe move this to a separate file
// Calculate pitch, roll, and heading.
// Pitch/roll calculations take from this app note:
// http://cache.freescale.com/files/sensors/doc/app_note/AN3461.pdf?fpsp=1
// Heading calculations taken from this app note:
// http://www51.honeywell.com/aero/common/documents/myaerospacecatalog-documents/Defense_Brochures-documents/Magnetic__Literature_Application_notes-documents/AN203_Compass_Heading_Using_Magnetometers.pdf
// READ: this also calculates heading (TODO remove later if unused)
void getAttitude(float& roll, float& pitch) {
    roll = atan2(imu.ay, imu.az);
    pitch = atan2(-imu.ax, sqrt(imu.ay * imu.ay + imu.az * imu.az));

    float heading;
    if (imu.my == 0)
        heading = (imu.mx < 0) ? PI : 0;
    else
        heading = atan2(imu.mx, imu.my);

    heading -= DECLINATION * PI / 180;

    if (heading > PI) heading -= (2 * PI);
    else if (heading < -PI) heading += (2 * PI);

    // Convert everything from radians to degrees:
    heading *= 180.0 / PI;
    pitch *= 180.0 / PI;
    roll  *= 180.0 / PI;
}

// sample devices for data
void sample() {
    reduced = false;

    // read altimeter 1
    if(alt1_online) {
        if(alt1.readDigitalValue()) {
            add_temp2(packet, alt1.getTemperature());
            if(high_alt) {
                add_alt(packet, alt1.getAltitude());
            }
        }
    }

    // read altimeter 2
    if(alt2_online) {
        if(!high_alt) {
            if(alt2.getAltitude(&packet->data.alt) == READ_SUCCESS) {
                set_alt_change(packet);
                if(packet->data.alt > ALT_SWITCH_THRESHOLD) {
                    high_alt = true;
                }
            }
        }
        if(alt2.getTemperature(&packet->data.temp1) == READ_SUCCESS) {
            set_temp1_change(packet);
        }
    }

    if(gps_online) {
        // read GPS
        if(gps.newNMEAreceived()) {
            if(gps.parse(gps.lastNMEA())) {
                add_lat(packet, gps.latitude);
                add_long(packet, gps.longitude);
                add_alt_gps(packet, gps.altitude); // meters above mean sea level
            }
        }
    }

    // update 200g accel
    accel200g.read(packet->data.x200g, packet->data.y200g, packet->data.z200g);
    set_200gaccel_change(packet);

    // update imu
    // all function calls are non-blocking (I think)
    if(imu_online) {
        imu.readAccel();
        add_16g_accel(packet, imu.calcAccel(imu.ax), imu.calcAccel(imu.ay), imu.calcAccel(imu.az));
        imu.readMag();
        add_16g_mag(packet, imu.calcMag(imu.mx), imu.calcMag(imu.my), imu.calcMag(imu.mz));
        imu.readGyro();
        getAttitude(packet->data.pitch, packet->data.roll);
        set_pitch_change(packet);
        set_roll_change(packet);
        //imu.readTemp(); //TODO potentially either log this or send it as temp2
        //add_temp2(packet, imu.temperature);
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
    if(!gps.begin(GPS_BAUD)) {
        gps_online = false;
        return;
    }
    gps.sendCommand(PMTK_SET_NMEA_UPDATE_5HZ);
    gps.sendCommand(PMTK_API_SET_FIX_CTL_5HZ);
    gps.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
    gps.sendCommand(PGCMD_ANTENNA);
}

void imu_init() {
    uint32_t timer = millis();
    uint32_t counter = 0;
    while(imu.beginSPI(NULL, NULL) == false) { // no CS pin (wired to 3.3v)
        if(counter >= 5) {
            imu_online = false;
            break;
        }
        if(millis() - timer >= 1000) {
            counter++;
        }
    }
}

void alt1_init() {
    uint32_t timer = millis();
    uint32_t counter = 0;
    while(!alt1.begin()) { // no CS pin (wired to 3.3v)
        if(counter >= 5) {
            alt1_online = false;
            break;
        }
        if(millis() - timer >= 1000) {
            counter++;
        }
    }
}

void alt2_init() {
    uint32_t timer = millis();
    uint32_t counter = 0;
    while(!alt2.begin()) { // no CS pin (wired to 3.3v)
        if(counter >= 5) {
            alt2_online = false;
            break;
        }
        if(millis() - timer >= 1000) {
            counter++;
        }
    }
}

void init() {
    gps_init();
    imu_init();
    alt1_init();
    alt2_init();
    xbee.begin(XBEE_BAUD);
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
