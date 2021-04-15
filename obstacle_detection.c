#include "sensors/proximity.h"
#include "sensors/VL53L0X/VL53L0X.h"

static THD_FUNCTION(obstacle_detection_thd, arg) {
     (void) arg;
     chRegSetThreadName(__FUNCTION__);

void obstacle_detection_start(void) {

	proximity_start();
	VL53L0X_start();

	static THD_WORKING_AREA(obstacle_detection_thd_wa, 1024);
	imuThd = chThdCreateStatic(obstacle_detection_thd_wa, sizeof(obstacle_detection_thd_wa),
			NORMALPRIO, obstacle_detection_thd, NULL);
}

