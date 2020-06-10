/*
Will Merges
*/
//ADXL377 200g accelerometer

#ifndef ADXL377_H
#define ADXL377_H

#define MIN_READ 0
#define MAX_READ 1023
#define MAX_G 200
#define MIN_G -200

class ADXL377 {
private:
    int x_pin;
    int y_pin;
    int z_pin;
public:
    ADXL377(int x_pin, int y_pin, int z_pin);
    void read(float& x, float& y, float& z);
    float map_output(int val);
};

#endif
