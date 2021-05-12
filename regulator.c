/*
 * 	regulator.c
 *
 *	Course: Embedded Systems and Robotics, MICRO-315
 *  Created on: April 12, 2021
 *  Author: Robin Bonny (283196) and Andrea Bruder (283199)
 */

#include <ch.h>
#include <hal.h>
#include <stdlib.h>
#include <chprintf.h>
#include <math.h>

#include <regulator.h>
#include <main.h>
#include <process_image.h>
#include <blinker.h>
#include <motors.h>
#include <mapping.h>

#include <leds.h>
#include <prox_sensors.h>

// PI Controller parameters
#define KP 3.5
#define KI 0.3

// Minimum error before actuating motors
#define THRESHOLD_ERR 5

// Limitation of errors for PI controller
#define WINDUP 40

// Threshold to initiate turn [mm]
#define FRONT_THRESHOLD 44

// Threshold for randomized turning
#define RAND_THRESHOLD 100

// Threshold to disable lateral controller [mm]
#define FRONT_DISABLE 170
// Threshold to disable lateral controller [IR]
#define IR_THRESHOLD_1 100
// Threshold to determine unobstructed path when turning [IR]
#define IR_THRESHOLD_2 150

// Number of indicator flashes
#define BLINKER 3

#define LATERAL_REGULATOR_PERIOD 20
#define FRONTAL_REGULATOR_PERIOD 60

static THD_WORKING_AREA(LateralRegulator_wa, 4096);
static THD_FUNCTION(LateralRegulator, arg) {
	(void) arg;
	chRegSetThreadName(__FUNCTION__);

	systime_t time;
	int32_t err = 0;
	int32_t integ = 0;
	int rightIR = 0;
	int leftIR = 0;

	while (1) {
		time = chVTGetSystemTime();
		// Average output of lateral and frontal-lateral IR sensors for controller
		rightIR = (get_TOFIR_values().IR_r_prox + get_TOFIR_values().IR_rf_prox)
				/ 2;
		leftIR = (get_TOFIR_values().IR_l_prox + get_TOFIR_values().IR_lf_prox)
				/ 2;
		// Computation of error and integral term for PI controller
		err = rightIR - leftIR;
		integ += err;

		// Limit errors to avoid excessive orientation changes
		if (integ > WINDUP)
			integ = WINDUP;
		else if (integ < -WINDUP)
			integ = -WINDUP;
		if (err > WINDUP)
			err = WINDUP;
		else if (err < -WINDUP)
			err = -WINDUP;

		// Disable controller when approaching obstacle or without reference on both sides
		if (get_TOFIR_values().TOF_dist < FRONT_DISABLE
				|| get_TOFIR_values().IR_r_prox < IR_THRESHOLD_1
				|| get_TOFIR_values().IR_l_prox < IR_THRESHOLD_1) {
			err = 0;
			integ = 0;
		}

		// Actuate motors
		if (abs(err) > THRESHOLD_ERR) {
			right_motor_set_speed(
			MOTORSPEED + err * KP + integ * KI);
			left_motor_set_speed(
			MOTORSPEED - err * KP - integ * KI);
		} else {
			right_motor_set_speed(MOTORSPEED);
			left_motor_set_speed(MOTORSPEED);
		}

		chThdSleepUntilWindowed(time, time + MS2ST(LATERAL_REGULATOR_PERIOD));
	}
}

static THD_WORKING_AREA(FrontalRegulator_wa, 1024);
static THD_FUNCTION(FrontalRegulator, arg) {
	(void) arg;
	chRegSetThreadName(__FUNCTION__);

	systime_t time;
	direction dir;

	while (1) {
		time = chVTGetSystemTime();

		// Initiate 90 degree turn when close to obstacle
		if (get_TOFIR_values().TOF_dist < FRONT_THRESHOLD) {
			dir = determine90();
			call_blinker(dir, BLINKER);
			set_turn(dir);
			motor_turn(dir, 90);
			motor_straight();
		}
		chThdSleepUntilWindowed(time, time + MS2ST(FRONTAL_REGULATOR_PERIOD));
	}
}

void lateral_regulator_start(void) {
	chThdCreateStatic(LateralRegulator_wa, sizeof(LateralRegulator_wa),
	NORMALPRIO - 1, LateralRegulator, NULL);
}

void frontal_regulator_start(void) {
	blinker_start();
	chThdCreateStatic(FrontalRegulator_wa, sizeof(FrontalRegulator_wa),
	NORMALPRIO, FrontalRegulator, NULL);
}

/*
 * @brief	Determines direction in which to turn by checking lateral IR sensors for obstructions.
 * 			Chooses a random direction when both sides are free.
 *
 * @return	Direction
 */
direction determine90(void) {
	direction dir;
	if (get_TOFIR_values().IR_r_prox > IR_THRESHOLD_2) {			// Left case
		dir = LEFT;
	} else if (get_TOFIR_values().IR_l_prox > IR_THRESHOLD_2) {		// Right case
		dir = RIGHT;
	} else {														// Random case
		systime_t time = chVTGetSystemTime();
		srand(time);
		(rand() % RAND_THRESHOLD > RAND_THRESHOLD / 2) ? (dir = LEFT) : (dir =
																	RIGHT);
	}
	return dir;
}
