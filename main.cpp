/*
Will Merges
*/
#define DEBUG

#include "Arduino.h"
#include "Adafruit_MPL3115A2.h"
#include "rf_protocol/rf_packet.h"
#include "charge.h"

#define BAUD 9600
#define xbee Serial4 // what serial the xbee radio is connected to

#ifdef INTERVAL_INTERRUPT
#define DELTA_TIME 10^6 //in microseconds
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
IntervalTimer uptimer;
IntervalTimer sendtimer;
Charge charges[4] {1,2,3,4};

//TODO check for roll over
// is that necessary?
void update_clock() {
   packet->data.uptime = uptime++;
   set_uptime_change(packet);
#ifndef INTERVAL_INTERRUPT
    temptime = millis();
#endif
}

//TODO maybe implement a sample rate (in main loop, interrupts probably too slow)
// sample devices for data
void sample() {
    reduced = false;
    if(alt2.getAltitude(&packet->data.alt) == READ_SUCCESS) {
        set_alt_change(packet);
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

void init() {
    xbee.begin(BAUD);
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
