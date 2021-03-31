#include "LSM6DS33.h"
#include "i2c.h"
#include "assert.h"
#include "printf.h"
#include "timer.h"

/*
 * Module to interact with the LSM6DS33 6DOF IMU over I2C. This is designed for use 
 * with the Adafruit LSM6DS33 breakout https://www.adafruit.com/product/4480 but would 
 * probably work with other boards with this sensor.
 * 
 * Author: Parthiv Krishna <parthiv@stanford.edu>
 * Date:   June 2020
 */

static int dual_sensors;
static int addresses[2];
static lsm6ds33_sensor_id_t active_sensor;
static double accel_multiplier;
static double gyro_multiplier;
static const double grav_accel = 9.80665;

void lsm6ds33_init(int addr, lsm6ds33_data_rate_t rate) {
    i2c_init();
    dual_sensors = 0;
    addresses[0] = addr;
    addresses[1] = 0;
    active_sensor = LSM6DS33_SENSOR0;
    
    // confirm sensor is connected
    char whoami = lsm6ds33_read_register(LSM6DS33_WHOAMI);    
    int sensor_connected = (whoami == 0x69);  // this register always contains 0x69
    assert(sensor_connected); // give it a nice name so that it makes sense if it fails
    lsm6ds33_write_register(LSM6DS33_CTRL9_XL, 0b00111000); // enable accel XYZ axes
    lsm6ds33_write_register(LSM6DS33_CTRL10_C, 0b00111000); // enable gyro XYZ axes
    lsm6ds33_set_accel_data_rate(rate);
    lsm6ds33_set_gyro_data_rate(rate);
    lsm6ds33_set_accel_range(LSM6DS33_ACCEL_RANGE_4G);
    lsm6ds33_set_gyro_range(LSM6DS33_GYRO_RANGE_250_DPS);
}

void lsm6ds33_init_dual(int addr0, int addr1, lsm6ds33_data_rate_t rate) {
    i2c_init();
    dual_sensors = 1;
    addresses[0] = addr0;
    addresses[1] = addr1;

    // confirm sensor0 is connected
    lsm6ds33_set_active_sensor(LSM6DS33_SENSOR0);
    char whoami = lsm6ds33_read_register(LSM6DS33_WHOAMI);
    int sensor0_connected = (whoami == 0x69);  // this register always contains 0x69
    assert(sensor0_connected); // give it a nice name so that it makes sense if it fails
    lsm6ds33_write_register(LSM6DS33_CTRL9_XL, 0b00111000); // enable accel XYZ axes
    lsm6ds33_write_register(LSM6DS33_CTRL10_C, 0b00111000); // enable gyro XYZ axes
    lsm6ds33_set_accel_data_rate(rate);
    lsm6ds33_set_gyro_data_rate(rate);
    lsm6ds33_set_accel_range(LSM6DS33_ACCEL_RANGE_4G);
    lsm6ds33_set_gyro_range(LSM6DS33_GYRO_RANGE_250_DPS);

    // confirm sensor1 is connected
    lsm6ds33_set_active_sensor(LSM6DS33_SENSOR1);    
    whoami = lsm6ds33_read_register(LSM6DS33_WHOAMI);
    int sensor1_connected = (whoami == 0x69);  // this register always contains 0x69
    assert(sensor1_connected); // give it a nice name so that it makes sense if it fails
    lsm6ds33_write_register(LSM6DS33_CTRL9_XL, 0b00111000); // enable accel XYZ axes
    lsm6ds33_write_register(LSM6DS33_CTRL10_C, 0b00111000); // enable gyro XYZ axes
    lsm6ds33_set_accel_data_rate(rate);
    lsm6ds33_set_gyro_data_rate(rate);
    lsm6ds33_set_accel_range(LSM6DS33_ACCEL_RANGE_4G);
    lsm6ds33_set_gyro_range(LSM6DS33_GYRO_RANGE_250_DPS);
}

void lsm6ds33_set_active_sensor(lsm6ds33_sensor_id_t id) {
    if (dual_sensors) { // change active_sensor if in dual sensor mode
        active_sensor = id;
    }
}

unsigned int lsm6ds33_set_accel_data_rate(lsm6ds33_data_rate_t rate) {
    char data = lsm6ds33_read_register(LSM6DS33_CTRL1_XL);
    data &= 0b00001111; // clear first four bits (data rate)
    data |= (rate << 4); // set the first four bits (data rate) according to requested rate 
    return lsm6ds33_write_register(LSM6DS33_CTRL1_XL, data);
}

unsigned int lsm6ds33_set_gyro_data_rate(lsm6ds33_data_rate_t rate) {
    char data = lsm6ds33_read_register(LSM6DS33_CTRL2_G);
    data &= 0b00001111; // clear first four bits (data rate)
    data |= (rate << 4); // set the first four bits (data rate) according to requested rate 
    return lsm6ds33_write_register(LSM6DS33_CTRL2_G, data);
}

unsigned int lsm6ds33_set_accel_range(lsm6ds33_accel_range_t range) {  
    switch (range) {
        case LSM6DS33_ACCEL_RANGE_16G:
            accel_multiplier = 0.0488; // 16000 millig / (10*2^15)
            break;
        case LSM6DS33_ACCEL_RANGE_8G:
            accel_multiplier = 0.0244; //  8000 millig / (10*2^15)
            break;
        case LSM6DS33_ACCEL_RANGE_4G:
            accel_multiplier = 0.0122; //  4000 millig / (10*2^15)
            break;
        case LSM6DS33_ACCEL_RANGE_2G:
            accel_multiplier = 0.0061; //  2000 millig / (10*2^15)
            break;
    }
    char data = lsm6ds33_read_register(LSM6DS33_CTRL1_XL);
    data &= 0b11110011; // clear two bits (data range)
    data |= (range << 2); // set the two bits (data range) according to requested range 
    return lsm6ds33_write_register(LSM6DS33_CTRL1_XL, data);
}

unsigned int lsm6ds33_set_gyro_range(lsm6ds33_gyro_range_t range) {
    switch (range) {
        case LSM6DS33_GYRO_RANGE_2000_DPS:
            gyro_multiplier = 70.0;
            break;
        case LSM6DS33_GYRO_RANGE_1000_DPS:
            gyro_multiplier = 35.0;
            break;
        case LSM6DS33_GYRO_RANGE_500_DPS:
            gyro_multiplier = 17.5;
            break;
        case LSM6DS33_GYRO_RANGE_250_DPS:
            gyro_multiplier = 8.75;
            break;
        case LSM6DS33_GYRO_RANGE_125_DPS:
            gyro_multiplier = 4.375;
            break;
    }
    char data = lsm6ds33_read_register(LSM6DS33_CTRL2_G);
    data &= 0b11110000; // clear last four bits (data range)
    data |= range; // set last four bits (data range) according to requested range 
    return lsm6ds33_write_register(LSM6DS33_CTRL2_G, data);
}

void lsm6ds33_get_all(lsm6ds33_data_t *data) {
    char buf[12];
    char reg = LSM6DS33_OUTX_L_G;
    i2c_write(addresses[active_sensor], &reg, 1);
    i2c_read(addresses[active_sensor], buf, 12);

    short gyrox = buf[1] << 8 | buf[0];
    short gyroy = buf[3] << 8 | buf[2];
    short gyroz = buf[5] << 8 | buf[4];
    short accelx = buf[7] << 8 | buf[6];
    short accely = buf[9] << 8 | buf[8];
    short accelz = buf[11] << 8 | buf[10];

    data->gyrox = gyrox * gyro_multiplier / 1000;
    data->gyroy = gyroy * gyro_multiplier / 1000;
    data->gyroz = gyroz * gyro_multiplier / 1000;

    data->accelx = accelx * accel_multiplier * grav_accel;
    data->accely = accely * accel_multiplier * grav_accel;
    data->accelz = accelz * accel_multiplier * grav_accel;
}

unsigned int lsm6ds33_get_accel_single_axis(lsm6ds33_axis_t axis) {
    char reg_l = LSM6DS33_OUTX_L_XL + 2*axis;
    char reg_h = reg_l + 1;
    int data = lsm6ds33_read_register(reg_l);
    data |= lsm6ds33_read_register(reg_h) << 8;
    return data;
}

unsigned int lsm6ds33_get_gyro_single_axis(lsm6ds33_axis_t axis) {
    char reg_l = LSM6DS33_OUTX_L_G + 2*axis; // X axis offset 0, Y axis offset 2, Z axis offset 4
    char reg_h = reg_l + 1;
    int data = lsm6ds33_read_register(reg_l);
    data |= lsm6ds33_read_register(reg_h) << 8;
    return data; 
}

char lsm6ds33_read_register(char reg) {
    char result = 0;
    i2c_write(addresses[active_sensor], &reg, 1);
    i2c_read(addresses[active_sensor], &result, 1);
    return result;
}

unsigned int lsm6ds33_write_register(char reg, char data) {
    char towrite[2] = {reg, data};
    i2c_write(addresses[active_sensor], towrite, 2);
    char result = lsm6ds33_read_register(reg); // confirm that it was successfully written
    return (result == data);
}