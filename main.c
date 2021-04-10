#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ch.h"
#include "hal.h"
#include "memory_protection.h"
#include <usbcfg.h>
#include <main.h>
#include <chprintf.h>

#include <sensors/imu.h>
#include <motors.h>

#define KP 0.001

messagebus_t bus;
MUTEX_DECL(bus_lock);
CONDVAR_DECL(bus_condvar);

static void serial_start(void)
{
	static SerialConfig ser_cfg = {
	    115200,
	    0,
	    0,
	    0,
	};

	sdStart(&SD3, &ser_cfg); // UART3.
}

int main(void)
{

    halInit();
    chSysInit();
    mpu_init();
    messagebus_init(&bus, &bus_lock, &bus_condvar);

    //starts the serial communication
    serial_start();
    //starts the USB communication
    usb_start();

    imu_start();
    motors_init();

    systime_t time1, time2;
    time2 = chVTGetSystemTime();
    float err = 0;


    while(1){
    	time1 = chVTGetSystemTime();

    	err = KP * get_gyro_filtered(Z_AXIS,5);

    	time2 = time1;

    	chprintf((BaseSequentialStream *)&SDU1, "GyroErr %f\r\n", err);
    }
}

#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void)
{
    chSysHalt("Stack smashing detected");
}
