#ifndef LSM6DS33_H
#define LSM6DS33_H

/*
 * Module to interact with the LSM6DS33 6DOF IMU over I2C. This is designed for use
 * with the Adafruit LSM6DS33 breakout https://www.adafruit.com/product/4480 but would
 * probably work with other boards with this sensor.
 *
 * Author: Parthiv Krishna <parthiv@stanford.edu>
 * Date:   June 2020
 */

// I2C addresses
#define LSM6DS33_I2CADDR_DEFAULT 0x6A	// Default
#define LSM6DS33_I2CADDR_ALTERNATE 0x6B // Alternate (solder jumper on Adafruit breakout)
// Important registers
#define LSM6DS33_FUNC_CFG_ACCESS 0x01 // Enable embedded functions register
#define LSM6DS33_INT1_CTRL 0x0D		  // Interrupt 1 control register
#define LSM6DS33_INT2_CTRL 0x0E		  // Interrupt 2 control register
#define LSM6DS33_WHOAMI 0x0F		  // Chip ID register
#define LSM6DS33_CTRL1_XL 0x10		  // Main accelerometer config register
#define LSM6DS33_CTRL2_G 0x11		  // Main gyro config register
#define LSM6DS33_CTRL3_C 0x12		  // Main configuration register
#define LSM6DS33_CTRL8_XL 0x17		  // Accel hi/lo pass filter config register
#define LSM6DS33_CTRL9_XL 0x18 		  // Accel enable register
#define LSM6DS33_CTRL10_C 0x19		  // Main configuration register
#define LSM6DS33_WAKEUP_SRC 0x1B	  // Reason for wakeup register
#define LSM6DS33_OUT_TEMP_L 0x20	  // Lower temperature data register
#define LSM6DS33_OUT_TEMP_H 0x21	  // Higher temperature data register
#define LSM6DS33_OUTX_L_G 0x22	 	  // Gyro data registers (sequential)
#define LSM6DS33_OUTX_H_G 0x23
#define LSM6DS33_OUTY_L_G 0x24
#define LSM6DS33_OUTY_H_G 0x25
#define LSM6DS33_OUTZ_L_G 0x26
#define LSM6DS33_OUTZ_H_G 0x27
#define LSM6DS33_OUTX_L_XL 0x28		  // Accelerometer data registers (sequential)
#define LSM6DS33_OUTX_H_XL 0x29
#define LSM6DS33_OUTY_L_XL 0x30
#define LSM6DS33_OUTY_H_XL 0x31
#define LSM6DS33_OUTZ_L_XL 0x32
#define LSM6DS33_OUTZ_H_XL 0x33
#define LSM6DS33_TAP_CFG 0x58	 // Tap/pedometer configuration register
#define LSM6DS33_WAKEUP_THS 0x5B // Single and double-tap function threshold register
#define LSM6DS33_WAKEUP_DUR 0x5C // Free-fall, wakeup, timestamp and sleep mode duration register
#define LSM6DS33_MD1_CFG 0x5E	 // Functions routing on INT1 register

// Data rates (for accel and gyro)
typedef enum data_rate {
	LSM6DS33_RATE_POWERDOWN = 0,
	LSM6DS33_RATE_12_5_HZ,
	LSM6DS33_RATE_26_HZ,
	LSM6DS33_RATE_52_HZ,
	LSM6DS33_RATE_104_HZ,
	LSM6DS33_RATE_208_HZ,
	LSM6DS33_RATE_416_HZ,
	LSM6DS33_RATE_833_HZ,
	LSM6DS33_RATE_1_66_KHZ,
	LSM6DS33_RATE_3_33_KHZ,
	LSM6DS33_RATE_6_66_KHZ,
} lsm6ds33_data_rate_t;

// Accelerometer data ranges (measured in g's)
typedef enum accel_range {
	LSM6DS33_ACCEL_RANGE_2G = 0,
	LSM6DS33_ACCEL_RANGE_16G,
	LSM6DS33_ACCEL_RANGE_4G,
	LSM6DS33_ACCEL_RANGE_8G
} lsm6ds33_accel_range_t;

// Gyro data ranges (measured in degrees per second)
typedef enum gyro_range {
	LSM6DS33_GYRO_RANGE_125_DPS = 0b0010,
	LSM6DS33_GYRO_RANGE_250_DPS = 0b0000,
	LSM6DS33_GYRO_RANGE_500_DPS = 0b0100,
	LSM6DS33_GYRO_RANGE_1000_DPS = 0b1000,
	LSM6DS33_GYRO_RANGE_2000_DPS = 0b1100,
} lsm6ds33_gyro_range_t;

// High pass filter bandwidth values
typedef enum hpf_range {
	LSM6DS_HPF_ODR_DIV_50 = 0,
	LSM6DS_HPF_ODR_DIV_100,
	LSM6DS_HPF_ODR_DIV_9,
	LSM6DS_HPF_ODR_DIV_400,
} lsm6ds33_hp_filter_t;

// Used to access multiple sensors
typedef enum lsm6ds33_sensor_id {
	LSM6DS33_SENSOR0 = 0,
	LSM6DS33_SENSOR1
} lsm6ds33_sensor_id_t;

// Used to access a specific axis of accel or gyro
typedef enum lsm6ds33_axis {
	LSM6DS33_AXIS_X = 0,
	LSM6DS33_AXIS_Y = 1,
	LSM6DS33_AXIS_Z = 2,
} lsm6ds33_axis_t;

typedef struct lsm6ds33_data {
	double accelx;
	double accely;
	double accelz;
	double gyrox;
	double gyroy;
	double gyroz;
} lsm6ds33_data_t;

/* Initializes a single LSM6DS33 sensor at the given address
 * and with the given data rate.
 */
void lsm6ds33_init(int addr, lsm6ds33_data_rate_t rate);

/* Initializes two LSM6DS33 sensors at the given addresses
 * and with the given data rate.
 *
 */
void lsm6ds33_init_dual(int addr1, int addr2, lsm6ds33_data_rate_t rate);

/* Set the active sensor. Future calls to library functions
 * will use this sensor until this function is called again.
 * Has no effect if initialized in single-sensor mode.
 */
void lsm6ds33_set_active_sensor(lsm6ds33_sensor_id_t id);

/* Set the accelerometer data rate of the currently active sensor.
 * Returns 1 if successful, 0 if unsuccessful.
 */
unsigned int lsm6ds33_set_accel_data_rate(lsm6ds33_data_rate_t rate);

/* Set the gyro data rate of the currently active sensor.
 * Returns 1 if successful, 0 if unsuccessful.
 */
unsigned int lsm6ds33_set_gyro_data_rate(lsm6ds33_data_rate_t rate);

/* Set the accelerometer range of the currently active sensor.
 * Returns 1 if successful, 0 if unsuccessful.
 */
unsigned int lsm6ds33_set_accel_range(lsm6ds33_accel_range_t range);

/* Set the gyro range of the currently active sensor.
 * Returns 1 if successful, 0 if unsuccessful.
 */
unsigned int lsm6ds33_set_gyro_range(lsm6ds33_gyro_range_t range);

/* Read all accelerometer and gyro axes. The data struct is populated
 * with gyro measurements in degrees per second and accelerometer
 * measurements in milli g's. 
 */
void lsm6ds33_get_all(lsm6ds33_data_t *data);

/* Read the accelerometer on the given axis */
unsigned int lsm6ds33_get_accel_single_axis(lsm6ds33_axis_t axis);

/* Read the accelerometer on the given axis */
unsigned int lsm6ds33_get_gyro_single_axis(lsm6ds33_axis_t axis);

/* Reads a single register from the currently active sensor.
 * Returns the value in that register.
 */
char lsm6ds33_read_register(char reg);

/* Writes a single register to the currently active sensor.
 * Returns 1 if successful, 0 if unsuccessful.
 */
unsigned int lsm6ds33_write_register(char reg, char data);

#endif