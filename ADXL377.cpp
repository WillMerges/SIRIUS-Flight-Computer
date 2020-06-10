/*
Will Merges
*/
#include "Arduino.h"
#include "ADXL377.h"

ADXL377::ADXL377(int x_pin, int y_pin, int z_pin) {
    this->x_pin = x_pin;
    this->y_pin = y_pin;
    this->z_pin = z_pin;
}

void ADXL377::read(float& x, float& y, float& z) {
    x = map_output(analogRead(x));
    y = map_output(analogRead(y));
    z = map_output(analogRead(z));
}

float ADXL377::map_output(int val) {
    return (float(val)-float(MIN_READ)) * float(MAX_G-MIN_G) / float(MAX_READ-MIN_READ) + MIN_G;
}
