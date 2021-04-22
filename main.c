#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ch.h"
#include "hal.h"
#include "memory_protection.h"
#include <usbcfg.h>
#include <main.h>
#include <chprintf.h>
#include <motors.h>

#include "obstacle_detection.h"
#include "motor_control.h"

static void serial_start(void) {
	static SerialConfig ser_cfg = { 115200, 0, 0, 0, };

	sdStart(&SD3, &ser_cfg); // UART3.
}

int main(void) {

	halInit();
	chSysInit();
	//mpu_init();
	motors_init();

	//starts the serial communication
	serial_start();
	//starts the USB communication
	usb_start();

	obstacle_detection_start();
	frontal_regulator_start();


	while (1) {
		/*
		 * 1. Read out sensors
		 * 2. Calculate action
		 * 3. Actuate motors
		 * 4. Calculate map
		 * 5. Transmit map
		 */
		chThdSleepMilliseconds(500);


	}
}

#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void) {
	chSysHalt("Stack smashing detected");
}
