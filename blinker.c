#include <ch.h>
#include <hal.h>
#include <stdlib.h>

#include <blinker.h>
#include <leds.h>

#define PARTYPERIOD 50
#define PARTIES 5

static bool leftBlinker = false;
static bool rightBlinker = false;
static bool frontBlinker = false;
static bool partyBlinker = false;
static uint8_t repeat = 0;

static THD_WORKING_AREA(blinker_thd_wa, 256);
static THD_FUNCTION(blinker_thd, arg) {
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
		chThdSleepMilliseconds(50);
	}
}

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

void party_blinker() {
	partyBlinker = true;
}

void blinker_start(void) {
	chThdCreateStatic(blinker_thd_wa, sizeof(blinker_thd_wa),
	NORMALPRIO, blinker_thd, NULL);
}
