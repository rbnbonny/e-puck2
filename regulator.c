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

#define KP 0.9//1.2
#define KI 0.3//0.3//0.2//0.0018//.00008//.001


#define DIFFSPEED 5
#define THRESHOLD_ERR 5

#define WINDUP 100

#define FRONT_THRESHOLD 44
#define RAND_THRESHOLD 100

#define IR_THRESHOLD_1 100
#define IR_THRESHOLD_2 150

//#define IR_THRESHOLD_UNI 270

#define LATERAL_REGULATOR_PERIOD 20
#define FRONTAL_REGULATOR_PERIOD 60

static int last_IR_l = 0;
static int last_IR_r = 0;

static THD_WORKING_AREA(lateral_regulator_thd_wa, 8192);
static THD_FUNCTION(lateral_regulator_thd, arg) {
	(void) arg;
	chRegSetThreadName(__FUNCTION__);

	systime_t time;
	int32_t err = 0;
	int rightIR = 0;
	int leftIR = 0;
	int32_t integ = 0;

	static uint8_t controller_mode = 0;

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

//		chprintf((BaseSequentialStream *) &SD3, "Integ %d ", integ);
//		chprintf((BaseSequentialStream *) &SD3, "Error %d \r\n", err);

		if (get_TOFIR_values().TOF_dist
				< 160 || get_TOFIR_values().IR_r_prox < IR_THRESHOLD_1 || get_TOFIR_values().IR_l_prox < IR_THRESHOLD_1) {
			err = 0;
			integ = 0;
//			controller_mode = 1;
//		} else if (leftIR > IR_THRESHOLD_1 && rightIR > IR_THRESHOLD_1) {
////			err = 0;
////			integ = 0;
//			controller_mode = 0;
		}

//		switch (controller_mode) {
//		case 0:
//			err = rightIR - leftIR;
//			integ += err;
//			last_IR_l = get_TOFIR_values().IR_l_prox;
//			last_IR_r = get_TOFIR_values().IR_r_prox;
//			set_led(LED5, 1);
//			set_led(LED1, 0);
//			break;
//		case 1:
//			if (leftIR < IR_THRESHOLD_1) {
//				err = (last_IR_r - get_TOFIR_values().IR_r_prox) / 2;
//				integ = 0;
////					chprintf((BaseSequentialStream *) &SD3, "Error %d \r\n", err);
//			} else if (rightIR < IR_THRESHOLD_1) {
//				err = (last_IR_l - get_TOFIR_values().IR_l_prox) / 2;
//				integ = 0;
////					chprintf((BaseSequentialStream *) &SD3, "Error %d \r\n", err);
//			}
//			set_led(LED5, 0);
//			set_led(LED1, 1);
//			break;
//		case 2:
//			err = 0;
//			integ = 0;
//			break;
//		default:
//			break;
//		}

//		else if (leftIR < IR_THRESHOLD_1 /*&& get_TOFIR_values().TOF_dist < 150*/) {
//			chprintf((BaseSequentialStream *) &SD3, "Last L %d \r\n",
//					last_IR_l);
//			err = (last_IR_l - get_TOFIR_values().IR_l_prox);
//			integ += err;
//		} else if (rightIR < IR_THRESHOLD_1 /*&& get_TOFIR_values().TOF_dist < 150*/) {
//			chprintf((BaseSequentialStream *) &SD3, "Last R %d \r\n",
//					last_IR_r);
//			err = -(last_IR_r - get_TOFIR_values().IR_r_prox);
//			integ += err;
//		} else {
//		}

//		else {
//		}

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

static THD_WORKING_AREA(frontal_regulator_thd_wa, 1024);
static THD_FUNCTION(frontal_regulator_thd, arg) {
	(void) arg;
	chRegSetThreadName(__FUNCTION__);

	systime_t time;
	direction dir;

	while (1) {
		time = chVTGetSystemTime();
		if (get_TOFIR_values().TOF_dist < FRONT_THRESHOLD) {
//			if (get_barcode_number() > 0) {
//				chprintf((BaseSequentialStream *) &SD3, "Code: %d \r\n",
//						get_barcode_number());
//				switch (get_barcode_number()) {
//				case 1:
//					motor_turn(RIGHT, 360);
//					break;
//				case 2:
//					motor_turn(LEFT, 360);
//					break;
//				default:
//					break;
//				}
//				motor_straight();
//			} else {
			dir = determine90();
			call_blinker(dir);
			set_turn(dir);
			motor_turn(dir, 90);
			chprintf((BaseSequentialStream *) &SD3, "       HALT \r\n");
			last_IR_l = get_TOFIR_values().IR_l_prox;
			last_IR_r = get_TOFIR_values().IR_r_prox;
			motor_straight();
//		}
		}

//			if (rand() % RAND_THRESHOLD > RAND_THRESHOLD / 2) {
//				palClearPad(GPIOD, GPIOD_LED3);
//				motor_turn(RIGHT, 90);
//				palSetPad(GPIOD, GPIOD_LED3);
//			} else {
//				palClearPad(GPIOD, GPIOD_LED7);
//				motor_turn(LEFT, 90);
//				palSetPad(GPIOD, GPIOD_LED7);
//			}
//		}
		chThdSleepUntilWindowed(time, time + MS2ST(FRONTAL_REGULATOR_PERIOD));
	}
}

void lateral_regulator_start(void) {
	chThdCreateStatic(lateral_regulator_thd_wa,
			sizeof(lateral_regulator_thd_wa),
			NORMALPRIO - 1, lateral_regulator_thd, NULL);
}

void frontal_regulator_start(void) {
	blinker_start();
	chThdCreateStatic(frontal_regulator_thd_wa,
			sizeof(frontal_regulator_thd_wa),
			NORMALPRIO, frontal_regulator_thd, NULL);
}

direction determine90(void) {
	direction dir;
//	chprintf((BaseSequentialStream *) &SD3, "LEFT IR %d   RIGHT IR %d \r\n",
//			get_TOFIR_values().IR_l_prox, get_TOFIR_values().IR_r_prox);
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

