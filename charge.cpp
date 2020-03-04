/*
Will Merges
*/

#include "charge.h"

Charge::Charge(int charge_number) {
    switch(charge_number) {
        case 0:
            deploy_pin = CHARGE1_DEPLOY;
            sense_pin = CHARGE1_SENSE;
            indicator_pin = CHARGE1_INDICATE;
            break;
        case 1:
            deploy_pin = CHARGE2_DEPLOY;
            sense_pin = CHARGE2_SENSE;
            indicator_pin = CHARGE2_INDICATE;
            break;
        case 2:
            deploy_pin = CHARGE3_DEPLOY;
            sense_pin = CHARGE3_SENSE;
            indicator_pin = CHARGE3_INDICATE;
            break;
        case 3:
            deploy_pin = CHARGE4_DEPLOY;
            sense_pin = CHARGE4_SENSE;
            indicator_pin = CHARGE4_INDICATE;
            break;
    }
    pinMode(deploy_pin, OUTPUT);
    pinMode(sense_pin, INPUT);
    pinMode(indicator_pin, OUTPUT);
    digitalWriteFast(indicator_pin, HIGH);
}

_Bool Charge::check_continuity() {
    return (_Bool)digitalReadFast(sense_pin);
}

void Charge::deploy() {
    digitalWriteFast(deploy_pin, HIGH);
    deployed = true;
}
