#include "printf.h"
#include "uart.h"
#include <stdbool.h>
#include "LSM6DS33.h"
#include "timer.h"
#include "read_angle.h"
#include "config.h"

void main(void) {
    uart_init();
    printf("Hello, world!\n");
    // lsm6ds33_init(LSM6DS33_I2CADDR_DEFAULT, LSM6DS33_RATE_104_HZ);
    lsm6ds33_init_dual(LSM6DS33_I2CADDR_DEFAULT, LSM6DS33_I2CADDR_ALTERNATE, LSM6DS33_RATE_104_HZ);
    // This was just the setup I was using to test; your axes will probably be different
    accel_angle_reader_t reader = createAngleReader(CONFIG_HORIZ, CONFIG_VERT);
    unsigned int lastPrintTime = timer_get_ticks();
    while (1) {
        double time = timer_get_ticks();
        bool shouldPrint = time - lastPrintTime > 250000;
        lsm6ds33_data_t data;

        lsm6ds33_set_active_sensor(LSM6DS33_SENSOR0);
        lsm6ds33_get_all(&data);
        if (shouldPrint) {
            printf("Sensor 0: \n");
            printf("Accel X: %04d Y: %04d Z: %04d\n", (int) data.accelx, (int) data.accely, (int) data.accelz);
            printf("Gyro  X: %04d Y: %04d Z: %04d\n", (int) data.gyrox, (int) data.gyroy, (int) data.gyroz);
        }
        lsm6ds33_set_active_sensor(LSM6DS33_SENSOR1);
        lsm6ds33_get_all(&data);
        if (shouldPrint) {
            printf("Sensor 1: \n");
            printf("Accel X: %04d Y: %04d Z: %04d\n", (int) data.accelx, (int) data.accely, (int) data.accelz);
            printf("Gyro  X: %04d Y: %04d Z: %04d\n\n", (int) data.gyrox, (int) data.gyroy, (int) data.gyroz);
        }
        updateAngle(&reader, &data);
        // Print every 250 ms
        // We don't want to use a delay since then we get fewer samples for the angle,
        // making it more inaccurate
        if (shouldPrint) {
            printf("Angle (for sensor 1): %d\n", (int) reader.angle);
            lastPrintTime = time;
        }
    }
    uart_putchar(EOT);
}
