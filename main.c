#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ch.h"
#include "hal.h"
#include "memory_protection.h"
#include <usbcfg.h>
#include <main.h>
#include <motors.h>
#include <chprintf.h>

#include "obstacle_detection.h"
#include "motor_control.h"
#include "regulator.h"

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

	motor_straight();
	obstacle_detection_start();
	frontal_regulator_start();
	lateral_regulator_start();

	while (1) {
		/*
		 * 1. Read out sensors
		 * 2. Calculate action
		 * 3. Actuate motors
		 * 4. Calculate map
		 * 5. Transmit map
		 */
		chprintf((BaseSequentialStream *) &SD3, "Waiting\r\n");
		frontal_obstacle_wait();
		chprintf((BaseSequentialStream *) &SD3, "SP Reset\r\n");
		palClearPad(GPIOD, GPIOD_LED5);
		chThdSleepMilliseconds(100);
		palSetPad(GPIOD, GPIOD_LED5);

	}
}

#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void) {
	chSysHalt("Stack smashing detected");
}
