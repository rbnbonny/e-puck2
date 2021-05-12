#include <ch.h>
#include <hal.h>
#include <stdlib.h>
#include <chprintf.h>

#include <regulator.h>
#include <main.h>
#include <obstacle_detection.h>
#include <process_image.h>
#include <blinker.h>
#include <motors.h>
#include <mapping.h>

#include <leds.h>

#define KP 3.5 //1.2
#define KI 0.3//0.3//0.2//0.0018//.00008//.001

#define DIFFSPEED 1
#define THRESHOLD_ERR 5

#define WINDUP 40

#define FRONT_THRESHOLD 44
#define RAND_THRESHOLD 100

#define IR_THRESHOLD_1 90
#define IR_THRESHOLD_2 150

#define LATERAL_REGULATOR_PERIOD 20
#define FRONTAL_REGULATOR_PERIOD 60

static THD_WORKING_AREA(LateralRegulator_wa, 4096);
static THD_FUNCTION(LateralRegulator, arg) {
	(void) arg;
	chRegSetThreadName(__FUNCTION__);

	systime_t time;
	int32_t err = 0;
	int rightIR = 0;
	int leftIR = 0;
	int32_t integ = 0;

	while (1) {
		time = chVTGetSystemTime();
		rightIR = (get_TOFIR_values().IR_r_prox + get_TOFIR_values().IR_rf_prox)
				/ 2;
		leftIR = (get_TOFIR_values().IR_l_prox + get_TOFIR_values().IR_lf_prox)
				/ 2;
		err = rightIR - leftIR;
		integ += err;

		if (integ > WINDUP)
			integ = WINDUP;
		if (integ < -WINDUP)
			integ = -WINDUP;
		if (err > WINDUP)
			err = WINDUP;
		if (err < -WINDUP)
			err = -WINDUP;

		if (get_TOFIR_values().TOF_dist
				< 160|| get_TOFIR_values().IR_r_prox < IR_THRESHOLD_1 || get_TOFIR_values().IR_l_prox < IR_THRESHOLD_1) {
			err = 0;
			integ = 0;
		}

		if (err < -THRESHOLD_ERR) {
			right_motor_set_speed(
			MOTORSPEED + DIFFSPEED * err * KP + integ * KI);
			left_motor_set_speed(
			MOTORSPEED - DIFFSPEED * err * KP - integ * KI);
		} else if (err > THRESHOLD_ERR) {
			right_motor_set_speed(
			MOTORSPEED + DIFFSPEED * err * KP + integ * KI);
			left_motor_set_speed(
			MOTORSPEED - DIFFSPEED * err * KP - integ * KI);
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
		if (get_TOFIR_values().TOF_dist < FRONT_THRESHOLD) {
			dir = determine90();
			call_blinker(dir, 3);
			set_turn(dir);
			motor_turn(dir, 90);
			motor_straight();
		}
		chThdSleepUntilWindowed(time, time + MS2ST(FRONTAL_REGULATOR_PERIOD));
	}
}

void lateral_regulator_start(void) {
	chThdCreateStatic(LateralRegulator_wa,
			sizeof(LateralRegulator_wa),
			NORMALPRIO - 1, LateralRegulator, NULL);
}

void frontal_regulator_start(void) {
	blinker_start();
	chThdCreateStatic(FrontalRegulator_wa,
			sizeof(FrontalRegulator_wa),
			NORMALPRIO, FrontalRegulator, NULL);
}

direction determine90(void) {
	direction dir;
	if (get_TOFIR_values().IR_r_prox > IR_THRESHOLD_2) {
		dir = LEFT;
	} else if (get_TOFIR_values().IR_l_prox > IR_THRESHOLD_2) {
		dir = RIGHT;
	} else {
		systime_t time = chVTGetSystemTime();
		srand(time);
		(rand() % RAND_THRESHOLD > RAND_THRESHOLD / 2) ? (dir = LEFT) : (dir =
																	RIGHT);
	}
	return dir;
}
