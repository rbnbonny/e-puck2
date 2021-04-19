#include <ch.h>
#include <hal.h>
#include <chprintf.h>

#include "sensors/proximity.h"
#include "sensors/VL53L0X/VL53L0X.h"

static thread_t *obstacleThd;

static THD_FUNCTION(obstacle_detection_thd, arg) {
	(void) arg;
	chRegSetThreadName(__FUNCTION__);

	chThdSleepMilliseconds(100);

	while (1) {
		volatile uint16_t testTOF = VL53L0X_get_dist_mm();
		chprintf((BaseSequentialStream *) &SD3,"TOF Distance: %d mm \r\n", testTOF);
		chThdSleepMilliseconds(100);
	}
}

void obstacle_detection_start(void) {

//	proximity_start();
	VL53L0X_start();

	static THD_WORKING_AREA(obstacle_detection_thd_wa, 1024);
	obstacleThd = chThdCreateStatic(obstacle_detection_thd_wa,
			sizeof(obstacle_detection_thd_wa),
			NORMALPRIO, obstacle_detection_thd, NULL);
}

