#include "timer.h"
#include "math.h"
#include "read_angle.h"
#include "assert.h"
#include "printf.h"


// Constant between 0 and 1
// For more responsiveness, increase it closer to 1
// For more accuracy (in the long run), decrease it closer to 0
static const double KP = 0.99;

accel_angle_reader_t createAngleReader(axis_t horizontal, axis_t vertical) {
    accel_angle_reader_t reader;
    reader.angle = 0;
    reader.m_lastUpdateTime = timer_get_ticks();
    reader.hAxis = horizontal;
    reader.vAxis = vertical;
    reader.m_initialized = false;
    return reader;
}

static double getAxisValue(const lsm6ds33_data_t* data, axis_t axis) {
    axis_t ax = axis & ~AXIS_REVERSED;
    double result;
    if (ax == X_AXIS) result = data->accelx;
    else if (ax == Y_AXIS) result = data->accely;
    else if (ax == Z_AXIS) result = data->accelz;
    else {
        // Should never reach here
        assert(false);
        return 0.0;
    }
    return result * (axis & AXIS_REVERSED ? -1 : 1);
}

static double getAngleRate(const lsm6ds33_data_t* data, axis_t hAxis, axis_t vAxis) {
    axis_t hAx = hAxis & ~AXIS_REVERSED,
           vAx = vAxis & ~AXIS_REVERSED;
    // Get 3rd axis
    axis_t axis = (X_AXIS + Y_AXIS + Z_AXIS) - hAx - vAx;
    double result;
    if (axis == X_AXIS) result = data->gyrox;
    else if (axis == Y_AXIS) result = data->gyroy;
    else if (axis == Z_AXIS) result = data->gyroz;
    else {
        assert(false);
        return 0.0;
    }
    bool flipped = false;
    if (hAxis & AXIS_REVERSED) flipped = !flipped;
    if (vAxis & AXIS_REVERSED) flipped = !flipped;
    // If right handed cross product has a negative coordinate
    if ((hAx == X_AXIS && vAx == Z_AXIS) || (hAx == Y_AXIS && vAx == X_AXIS) || (hAx == Z_AXIS && vAx == Y_AXIS))
        flipped = !flipped;
    return result * (flipped ? -1 : 1);
}

void updateAngle(accel_angle_reader_t* reader, const lsm6ds33_data_t* data) {
    double x = getAxisValue(data, reader->hAxis);
    double y = getAxisValue(data, reader->vAxis);
    // From free body diagram
    double absoluteAngle = atan2(x, y) * DEGREES_PER_RADIAN;
    double time = timer_get_ticks();
    double deltaT = (time - reader->m_lastUpdateTime) / 1000000;  // time since last update in seconds
    double angleChange = getAngleRate(data, reader->hAxis, reader->vAxis) * deltaT;
    // Filtered angle
    if (reader->m_initialized) {
        reader->angle += KP * angleChange + (1 - KP) * (absoluteAngle - reader->angle);
    } else {
        reader->angle = absoluteAngle;
        reader->m_initialized = true;
    }
    reader->m_lastUpdateTime = time;
}