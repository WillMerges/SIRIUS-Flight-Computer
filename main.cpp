#define DEBUG

#include "Arduino.h"
#include "Adafruit_MPL3115A2.h"
#include "rf_protocol/rf_packet.h"

#define BAUD 9600
#define radio Serial4

// allocate mem
rf_data packet;
extern size_t packet_size;

Adafruit_MPL3115A2 alt2;

// sample devices for data
void sample() {
    //TODO figure out efficuent way to use non-blocking but not set update flag in packet every time
    //add data to packet here
}

// send data over radio
void send() {
    reduce_packet(packet);
    radio.write(packet->serialized, packet_size);
}

int main(int argc, char** argv) {
    radio.begin(BAUD);
    alt2.begin();
    packet = create_packet();

    while(1) {
        sample();
        send();

            yield();
    }

/**TODO
*   // test possible issue with saving of xyz variables
*   add_200g_accel(packet, 101, 102, 103);
*   reduce_packet(packet);
*   print_packet(packet);
*/
}
