#include "printf.h"
#include "uart.h"
#include "LSM6DS33.h"
#include "timer.h"

void main(void) {
    uart_init();
    printf("Hello, world!\n");
    // lsm6ds33_init(LSM6DS33_I2CADDR_DEFAULT, LSM6DS33_RATE_104_HZ);
    lsm6ds33_init_dual(LSM6DS33_I2CADDR_DEFAULT, LSM6DS33_I2CADDR_ALTERNATE, LSM6DS33_RATE_104_HZ);
    while (1) {
        lsm6ds33_set_active_sensor(LSM6DS33_SENSOR0);
        lsm6ds33_read_register(LSM6DS33_WHOAMI);        
        char whoami0 = lsm6ds33_read_register(LSM6DS33_WHOAMI);

        lsm6ds33_set_active_sensor(LSM6DS33_SENSOR1);
        lsm6ds33_read_register(LSM6DS33_WHOAMI);        
        char whoami1 = lsm6ds33_read_register(LSM6DS33_WHOAMI);

        printf("received 0x%x from sensor0 and 0x%x from sensor1\n", whoami0, whoami1); // expect 0x69 from both
        timer_delay(1);
    }
    uart_putchar(EOT);
}