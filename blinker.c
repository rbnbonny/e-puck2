#include <ch.h>
#include <hal.h>
#include <stdlib.h>

#include <blinker.h>
#include <leds.h>

static bool leftBlinker = false;
static bool rightBlinker = false;

void blinker(direction dir);

static THD_WORKING_AREA(blinker_thd_wa, 256);
static THD_FUNCTION(blinker_thd, arg) {
	(void) arg;
	chRegSetThreadName(__FUNCTION__);

	while (1) {
		if(leftBlinker){
			blinker(LEFT);
			leftBlinker = false;
		}
		if(rightBlinker){
			blinker(RIGHT);
			rightBlinker = false;
		}
		chThdSleepMilliseconds(50);
	}
}

void blinker(direction dir) {
	for (uint8_t i = 0; i < 3; i++) {

		switch (dir) {
		case LEFT:
			set_led(LED7, 1);
			break;
		case RIGHT:
			set_led(LED3, 1);
			break;
		default:
			break;
		}
		chThdSleepMilliseconds(100);
		clear_leds();
		chThdSleepMilliseconds(100);
	}
}

void call_blinker(direction dir){
	switch (dir) {
		case LEFT:
			leftBlinker = true;
			break;
		case RIGHT:
			rightBlinker = true;
			break;
		default:
			break;
	}
}

void blinker_start(void) {
	chThdCreateStatic(blinker_thd_wa, sizeof(blinker_thd_wa),
	NORMALPRIO, blinker_thd, NULL);
}
