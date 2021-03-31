#ifndef READ_ANGLE_H
#define READ_ANGLE_H

#include <stdbool.h>
#include "LSM6DS33.h"

// 180 / pi
#define DEGREES_PER_RADIAN 57.29577951308232
#define ANGLE_BUFFER_LEN 4
#define ANGLE_BUFFER_NUM_SAMPLES 10

enum Axes {
    X_AXIS = 0,
    Y_AXIS = 2,
    Z_AXIS = 4,
    AXIS_REVERSED = 1
};
typedef unsigned int axis_t;

enum GestureState {
    BEYOND_RANGE_FIRED,
    BEYOND_RANGE_NOT_FIRED,
    BEFORE_RANGE
};
typedef enum GestureState gesture_state_t;

struct gesture_handler {
    double angle;  // in degrees
    double omega;  // angular velocity
    double alpha;  // angular acceleration
    double angleBuffer[ANGLE_BUFFER_LEN];
    double calibration;
    unsigned int m_lastUpDownGestureTime;
    unsigned int m_lastUpdateTime;
    unsigned int angleBufferSamples;
    axis_t hAxis;
    axis_t vAxis;
    axis_t angleAxis;
    gesture_state_t gestureUpDownState;
    bool m_initialized;
};

typedef struct gesture_handler gesture_handler_t;


/**
 * @param horizontal This is the direction that you want to define as horizontal and forward
 *                   In other words, which of the axes (X, Y, or Z) points horizontally away from you
 *                   if you hold the stick?
 * @param vertical   This is the direction that should be considered up and vertical
 */
gesture_handler_t createGestureReader(axis_t horizontal, axis_t vertical);

void calibrate(gesture_handler_t* reader);

/**
 * The more frequently this is called, the more accurate it will be, generally.
 * If it is being called less frequently, you should change the KP constant in read_angle.c to something smaller
 */
void updateAngle(gesture_handler_t* reader, const lsm6ds33_data_t* data);

bool checkUpDownGesture(gesture_handler_t* reader);

#endif