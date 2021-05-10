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
#include "process_image.h"
#include "music.h"

static void serial_start(void) {
	static SerialConfig ser_cfg = { 115200, 0, 0, 0, };

	sdStart(&SD3, &ser_cfg); // UART3.
}

int main(void) {

	halInit();
	chSysInit();
	motors_init();

	//starts the serial communication
	serial_start();
	//starts the USB communication
	usb_start();

    dcmi_start();
	po8030_start();


	process_image_start();
	obstacle_detection_start();

	chThdSleepMilliseconds(1000);

	motor_straight();
	frontal_regulator_start();
	lateral_regulator_start();
	music_start();

	while (1) {
		/*
		 * 1. Read out sensors
		 * 2. Calculate action
		 * 3. Actuate motors
		 * 4. Calculate map
		 * 5. Transmit map
		 */
		chThdSleepMilliseconds(100);
	}
}

#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void) {
	chSysHalt("Stack smashing detected");
}
