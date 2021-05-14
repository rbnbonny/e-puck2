/*
 * 	main.c
 *
 *	Course: Embedded Systems and Robotics, MICRO-315
 *  Created on: April 08, 2021
 *  Author: Robin Bonny (283196) and Andrea Bruder (283199)
 */

#include <ch.h>
#include <hal.h>
#include <stdlib.h>
#include <memory_protection.h>
#include <usbcfg.h>

#include <motors.h>
#include <leds.h>

#include "main.h"
#include "prox_sensors.h"
#include "motor_control.h"
#include "regulator.h"
#include "process_image.h"
#include "mapping.h"
#include "music.h"

#define INITIAL_WAIT 2500

static void serial_start(void) {
	static SerialConfig ser_cfg = { 115200, 0, 0, 0, };
	sdStart(&SD3, &ser_cfg); // UART3.
}

int main(void) {

	halInit();
	chSysInit();
	motors_init();

	serial_start();
	usb_start();

    dcmi_start();
	po8030_start();

	process_image_start();

	chThdSleepMilliseconds(INITIAL_WAIT);

	obstacle_detection_start();

	chThdSleepMilliseconds(INITIAL_WAIT);

	frontal_regulator_start();
	lateral_regulator_start();
	mapping_start();
	music_start();

	motor_straight();

	while (1) {
		chThdSleepMilliseconds(100);
	}
}

#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void) {
	chSysHalt("Stack smashing detected");
}
