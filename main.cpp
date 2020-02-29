#define DEBUG

#include "Arduino.h"
//#include "Adafruit_MPL3115A2.h"
#include "rf_protocol/rf_packet.h"

#define BAUD 9600

//rf_data packet;
//extern size_t packet_size;

//void sample() {

//}

int main(int argc, char** argv) {
    //packet = create_packet();
    //add_200g_accel(packet, 101, 102, 103);
    //reduce_packet(packet);
    //print_packet(packet);

    Serial4.begin(BAUD);
//    Serial4.write(packet->serialized, packet_size);

    //sample();
    yield();
}
