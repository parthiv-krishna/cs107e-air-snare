#include "timer.h"
#include "math.h"
#include "read_angle.h"
#include "assert.h"
#include "printf.h"


// static const double UPDOWN_GESTURE_HIGH = 2,
//                     UPDOWN_GESTURE_LOW = 0;

static const double UPDOWN_GESTURE_POS_ACCEL_TOP = 8000;
static const double UPDOWN_GESTURE_POS_ACCEL_BOTTOM = 100;

// Constant between 0 and 1
// For more responsiveness, increase it closer to 1
// For more accuracy (in the long run), decrease it closer to 0
static const double KP_angle = 0.999;

static axis_t getAngleAxis(axis_t hAxis, axis_t vAxis) {
    axis_t hAx = hAxis & ~AXIS_REVERSED,
           vAx = vAxis & ~AXIS_REVERSED;
    // Get 3rd axis
    axis_t axis = (X_AXIS + Y_AXIS + Z_AXIS) - hAx - vAx;
    bool flipped = false;
    if (hAxis & AXIS_REVERSED) flipped = !flipped;
    if (vAxis & AXIS_REVERSED) flipped = !flipped;
    // If right handed cross product has a negative coordinate
    if ((hAx == X_AXIS && vAx == Z_AXIS) || (hAx == Y_AXIS && vAx == X_AXIS) || (hAx == Z_AXIS && vAx == Y_AXIS))
        flipped = !flipped;
    if (flipped) axis |= AXIS_REVERSED;
    return axis;
}


gesture_handler_t createGestureReader(axis_t horizontal, axis_t vertical) {
    gesture_handler_t reader;
    reader.angle = 0;
    reader.m_lastUpdateTime = timer_get_ticks();
    reader.gestureUpDownState = BEFORE_RANGE;
    reader.hAxis = horizontal;
    reader.vAxis = vertical;
    reader.angleAxis = getAngleAxis(horizontal, vertical);
    reader.omega = 0;
    reader.alpha = 0;
    reader.m_initialized = false;
    reader.angleBufferSamples = 0;
    reader.calibration = 0;
    for (size_t i = 0; i < ANGLE_BUFFER_LEN; ++i) reader.angleBuffer[i] = 0;
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

static double getAngleValue(const lsm6ds33_data_t* data, axis_t axis) {
    axis_t ax = axis & ~AXIS_REVERSED;
    double result;
    if (ax == X_AXIS) result = data->gyrox;
    else if (ax == Y_AXIS) result = data->gyroy;
    else if (ax == Z_AXIS) result = data->gyroz;
    else {
        // Should never reach here
        assert(false);
        return 0.0;
    }
    return result * (axis & AXIS_REVERSED ? -1 : 1);
}


void calibrate(gesture_handler_t* reader) {
    printf("Calibrating. Leave the stick still\n");
    const size_t numSamples = 1000;
    double total = 0;
    for (size_t i = 0; i < numSamples; ++i) {
        lsm6ds33_data_t data;
        lsm6ds33_get_all(&data);
        total += getAngleValue(&data, reader->angleAxis) / ((double)numSamples);
        timer_delay_ms(1);
    }
    reader->calibration = total;
}

void updateAngle(gesture_handler_t* reader, const lsm6ds33_data_t* data) {
    double x = getAxisValue(data, reader->hAxis);
    double y = getAxisValue(data, reader->vAxis);

    // From free body diagram
    double absoluteAngle = atan2(x, y) * DEGREES_PER_RADIAN;
    unsigned int time = timer_get_ticks();
    double deltaT = (time - reader->m_lastUpdateTime) / 1000000.0;  // time since last update in seconds
    double omega = getAngleValue(data, reader->angleAxis) - reader->calibration;
    double angleChange = omega * deltaT;
    // Filtered angle
    if (reader->m_initialized) {
        reader->angle += KP_angle * angleChange + (1 - KP_angle) * (absoluteAngle - reader->angle);
    } else {
        reader->angle = absoluteAngle;
        reader->m_initialized = true;
    }

    if (reader->gestureUpDownState == BEYOND_RANGE_FIRED /*&& reader->angle > UPDOWN_GESTURE_HIGH*/ 
        && reader->alpha < UPDOWN_GESTURE_POS_ACCEL_BOTTOM && time - reader->m_lastUpDownGestureTime > 120000) {
        reader->gestureUpDownState = BEFORE_RANGE;
    } else if (reader->gestureUpDownState == BEFORE_RANGE && reader->alpha > UPDOWN_GESTURE_POS_ACCEL_TOP 
                /*&& reader->angle < UPDOWN_GESTURE_LOW*/) {
        reader->gestureUpDownState = BEYOND_RANGE_NOT_FIRED;
        reader->m_lastUpDownGestureTime = time;
    }

    reader->angleBuffer[ANGLE_BUFFER_LEN - 1] += reader->angle / ANGLE_BUFFER_NUM_SAMPLES;
    if (++reader->angleBufferSamples == ANGLE_BUFFER_NUM_SAMPLES) {
        // Update alpha, omega
        // Source for coefficients: https://en.wikipedia.org/wiki/Finite_difference_coefficient#Forward_and_backward_finite_difference
        double alpha = 0, omega = 0;
        reader->angleBufferSamples = 0;
        static const double deriv2coeffs[ANGLE_BUFFER_LEN] = {-1, 4, -5, 2};
        for (size_t i = 0; i < ANGLE_BUFFER_LEN; ++i) alpha += deriv2coeffs[i] * reader->angleBuffer[i];
        double dt = deltaT * ANGLE_BUFFER_NUM_SAMPLES;
        alpha /= (dt * dt);
        static const double deriv1coeffs[ANGLE_BUFFER_LEN] = {-0.3333, 1.5, -3.0, 1.83333};
        for (size_t i = 0; i < ANGLE_BUFFER_LEN; ++i) omega += deriv1coeffs[i] * reader->angleBuffer[i];
        omega /= dt;
        for (size_t i = 0; i < ANGLE_BUFFER_LEN - 1; ++i) reader->angleBuffer[i] = reader->angleBuffer[i + 1];
        reader->angleBuffer[ANGLE_BUFFER_LEN - 1] = 0;
        reader->omega = omega;
        reader->alpha = alpha;
    }

    reader->m_lastUpdateTime = time;
}

bool checkUpDownGesture(gesture_handler_t* reader) {
    if (reader->gestureUpDownState == BEYOND_RANGE_NOT_FIRED) {
        reader->gestureUpDownState = BEYOND_RANGE_FIRED;
        return true;
    } else return false;
}