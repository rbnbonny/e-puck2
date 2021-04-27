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

#define IR_THRESHOLD 500

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
//		frontal_obstacle_wait();
//		switch (get_barcode_number()) {
//			case 0:
//				break;
//			case 1:
//				break;
//			case 2:
//				break;
//			case 3:
//				break;
//			case 4:
//				break;
//			case 5:
//				break;
//			case 6:
//				break;
//			case 7:
//				break;
//			case 8:
//				break;
//			case 9:
//				break;
//			default:
//				break;
//		}
		chprintf((BaseSequentialStream *) &SD3, "LEFT IR %d   RIGHT IR %d",
				get_TOFIR_values().IR_l_prox, get_TOFIR_values().IR_r_prox);

//		if (get_TOFIR_values().IR_l_prox < IR_THRESHOLD) {
//		chSysLock();
//		chSysUnlock();
		motor_turn(LEFT, 90);
//		}
//		if (get_TOFIR_values().IR_r_prox < IR_THRESHOLD) {
//			motor_turn(RIGHT, 90);
//		}
		chThdSleepMilliseconds(1000);

	}
}

#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void) {
	chSysHalt("Stack smashing detected");
}
