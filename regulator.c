#include <ch.h>
#include <hal.h>
#include <chprintf.h>

#include <regulator.h>
#include <main.h>
#include <obstacle_detection.h>
#include <motors.h>

#define KP 1
#define DIFFSPEED 5
#define THRESHOLD_ERR 80

static THD_WORKING_AREA(regulator_thd_wa, 1024);
static THD_FUNCTION(regulator_thd, arg) {
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

		chprintf((BaseSequentialStream *) &SD3, "Error: %d \r\n", err);

		if (err < -THRESHOLD_ERR) {
			right_motor_set_speed(MOTORSPEED + KP * DIFFSPEED * err);
			left_motor_set_speed(MOTORSPEED - KP * DIFFSPEED * err);
		} else if (err > THRESHOLD_ERR) {
			right_motor_set_speed(MOTORSPEED + KP * DIFFSPEED * err);
			left_motor_set_speed(MOTORSPEED - KP * DIFFSPEED * err);
		}

		chThdSleepUntilWindowed(time, time + MS2ST(10));

	}

}

void regulator_start(void) {
	chThdCreateStatic(regulator_thd_wa, sizeof(regulator_thd_wa),
	NORMALPRIO, regulator_thd, NULL);
}
