/*
Will Merges
*/

#include "Arduino.h"

#define CHARGE1_DEPLOY 13
#define CHARGE2_DEPLOY 14
#define CHARGE3_DEPLOY 15
#define CHARGE4_DEPLOY 16

#define CHARGE1_SENSE 36
#define CHARGE2_SENSE 35
#define CHARGE3_SENSE 34
#define CHARGE4_SENSE 33

#define CHARGE1_INDICATE 24
#define CHARGE2_INDICATE 25
#define CHARGE3_INDICATE 26
#define CHARGE4_INDICATE 27

class Charge {
public:
    Charge(int charge_number);
    _Bool check_continuity();
    void deploy();
private:
    uint8_t deploy_pin;
    uint8_t sense_pin;
    uint8_t indicator_pin;
    _Bool deployed;
};
