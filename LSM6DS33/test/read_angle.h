#ifndef READ_ANGLE_H
#define READ_ANGLE_H

#include <stdbool.h>
#include "LSM6DS33.h"

// 180 / pi
#define DEGREES_PER_RADIAN 57.29577951308232

enum Axes {
    X_AXIS = 0,
    Y_AXIS = 2,
    Z_AXIS = 4,
    AXIS_REVERSED = 1
};
typedef unsigned int axis_t;


struct accel_angle_reader {
    double angle;  // in degrees
    double m_lastUpdateTime;
    axis_t hAxis;
    axis_t vAxis;
    bool m_initialized;
};

typedef struct accel_angle_reader accel_angle_reader_t;


/**
 * @param horizontal This is the direction that you want to define as horizontal and forward
 *                   In other words, which of the axes (X, Y, or Z) points horizontally away from you
 *                   if you hold the stick?
 * @param vertical   This is the direction that should be considered up and vertical
 */
accel_angle_reader_t createAngleReader(axis_t horizontal, axis_t vertical);

/**
 * The more frequently this is called, the more accurate it will be, generally.
 * If it is being called less frequently, you should change the KP constant in read_angle.c to something smaller
 */
void updateAngle(accel_angle_reader_t* reader, const lsm6ds33_data_t* data);

#endif