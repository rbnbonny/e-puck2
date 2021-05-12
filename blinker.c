/*
 * 	blinker.c
 *
 *	Course: Embedded Systems and Robotics, MICRO-315
 *  Created on: April 12, 2021
 *  Author: Robin Bonny (283196) and Andrea Bruder (283199)
 */

#include <ch.h>
#include <hal.h>
#include <stdlib.h>

#include <blinker.h>
#include <leds.h>

#define BLINKER_WAIT 50
#define PARTYPERIOD 50
#define PARTIES 10

static bool leftBlinker = false;
static bool rightBlinker = false;
static bool frontBlinker = false;
static bool partyBlinker = false;
static uint8_t repeat = 0;

static THD_WORKING_AREA(Blinker_wa, 256);
static THD_FUNCTION(Blinker, arg) {
	(void) arg;
	chRegSetThreadName(__FUNCTION__);

	while (1) {
		if (leftBlinker) {
			blinker(LEFT);
			leftBlinker = false;
		}
		if (rightBlinker) {
			blinker(RIGHT);
			rightBlinker = false;
		}
		if (frontBlinker) {
			blinker(STRAIGHT);
			frontBlinker = false;
		}
		if (partyBlinker) {
			for (uint8_t i = 0; i < PARTIES; i++) {
				set_led(LED1, 1);
				chThdSleepMilliseconds(PARTYPERIOD);
				set_led(LED1, 0);
				set_body_led(1);
				chThdSleepMilliseconds(PARTYPERIOD);
				set_led(LED3, 1);
				set_body_led(0);
				chThdSleepMilliseconds(PARTYPERIOD);
				set_led(LED3, 0);
				set_body_led(1);
				chThdSleepMilliseconds(PARTYPERIOD);
				set_led(LED5, 1);
				set_body_led(0);
				chThdSleepMilliseconds(PARTYPERIOD);
				set_led(LED5, 0);
				set_body_led(1);
				chThdSleepMilliseconds(PARTYPERIOD);
				set_led(LED7, 1);
				set_body_led(0);
				chThdSleepMilliseconds(PARTYPERIOD);
				set_led(LED7, 0);
				set_body_led(1);
				chThdSleepMilliseconds(PARTYPERIOD);
				set_body_led(0);
			}
			partyBlinker = false;
		}
		chThdSleepMilliseconds(BLINKER_WAIT);
	}
}

/**
* @brief   			activates blinking of the robot, depending on a left or right turn
* @brief			activates blinking of the robot, depending on the seen barcode number (nb of blinking = nb of barcode)
*
* @param[in] dir	direction of the robot, indicates if turn blinking or barcode blinking
*/
void blinker(direction dir) {
	for (uint8_t i = 0; i < repeat; i++) {

		switch (dir) {
		case LEFT:
			set_led(LED7, 1);
			break;
		case RIGHT:
			set_led(LED3, 1);
			break;
		case STRAIGHT:
			set_led(LED1, 1);
			break;
		default:
			break;
		}
		chThdSleepMilliseconds(100);
		clear_leds();
		chThdSleepMilliseconds(100);
	}
}

/**
* @brief					initializes turn or barcode blinker
*
* @param[in] dir			direction of the robot
* @param[in] ext_repeat 	nb to declare blinking times
*/
void call_blinker(direction dir, uint8_t ext_repeat) {
	switch (dir) {
	case LEFT:
		leftBlinker = true;
		break;
	case RIGHT:
		rightBlinker = true;
		break;
	case STRAIGHT:
		frontBlinker = true;
		break;
	default:
		break;
	}
	repeat = ext_repeat;
}

/**
 * @brief 					initializes party_blinker
 */
void party_blinker() {
	partyBlinker = true;
}

void blinker_start(void) {
	chThdCreateStatic(Blinker_wa, sizeof(Blinker_wa),
	NORMALPRIO, Blinker, NULL);
}
