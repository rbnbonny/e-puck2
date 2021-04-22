#include <ch.h>
#include <hal.h>
#include <stdlib.h>
#include <chprintf.h>

#include <regulator.h>
#include <main.h>
#include <obstacle_detection.h>
#include <motor_control.h>

#define KP 1
#define DIFFSPEED 5
#define THRESHOLD_ERR 80

#define FRONT_THRESHOLD 40

#define RAND_THRESHOLD 100

#define LATERAL_REGULATOR_PERIOD 10
#define FRONTAL_REGULATOR_PERIOD 200

static THD_WORKING_AREA(lateral_regulator_thd_wa, 1024);
static THD_FUNCTION(lateral_regulator_thd, arg) {
	(void) arg;
	chRegSetThreadName(__FUNCTION__);

	systime_t time;
	int err = 0;
	int rightIR = 0;
	int leftIR = 0;

	while (1) {
		time = chVTGetSystemTime();
		rightIR = (get_TOFIR_values().IR_r_prox + get_TOFIR_values().IR_rf_prox)
				/ 2;
		leftIR = (get_TOFIR_values().IR_l_prox + get_TOFIR_values().IR_lf_prox)
				/ 2;
		err = rightIR - leftIR;

//		chprintf((BaseSequentialStream *) &SD3, "Error: %d \r\n", err);

		if (err < -THRESHOLD_ERR) {
			right_motor_set_speed(MOTORSPEED + KP * DIFFSPEED * err);
			left_motor_set_speed(MOTORSPEED - KP * DIFFSPEED * err);
		} else if (err > THRESHOLD_ERR) {
			right_motor_set_speed(MOTORSPEED + KP * DIFFSPEED * err);
			left_motor_set_speed(MOTORSPEED - KP * DIFFSPEED * err);
		}

		chThdSleepUntilWindowed(time, time + MS2ST(LATERAL_REGULATOR_PERIOD));
	}
}

static THD_WORKING_AREA(frontal_regulator_thd_wa, 1024);
static THD_FUNCTION(frontal_regulator_thd, arg) {
	(void) arg;
	chRegSetThreadName(__FUNCTION__);

	systime_t time;

	while (1) {
		time = chVTGetSystemTime();
		chprintf((BaseSequentialStream *) &SD3, "TOF Distance: %d mm \r\n",
				get_TOFIR_values().TOF_dist);
		if (get_TOFIR_values().TOF_dist < FRONT_THRESHOLD) {
			srand(time);
			if (rand() % RAND_THRESHOLD > RAND_THRESHOLD / 2) {
				palClearPad(GPIOD, GPIOD_LED3);
				motor_turn(RIGHT, 90);
				palSetPad(GPIOD, GPIOD_LED3);
			} else {
				palClearPad(GPIOD, GPIOD_LED7);
				motor_turn(LEFT, 90);
				palSetPad(GPIOD, GPIOD_LED7);
			}
		}
		chThdSleepMilliseconds(100);

		chThdSleepUntilWindowed(time, time + MS2ST(FRONTAL_REGULATOR_PERIOD));
	}
}

void lateral_regulator_start(void) {
	chThdCreateStatic(lateral_regulator_thd_wa,
			sizeof(lateral_regulator_thd_wa),
			NORMALPRIO, lateral_regulator_thd, NULL);
}

void frontal_regulator_start(void) {
	chThdCreateStatic(frontal_regulator_thd_wa,
			sizeof(frontal_regulator_thd_wa), NORMALPRIO, frontal_regulator_thd,
			NULL);
}
