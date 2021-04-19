#include <ch.h>
#include <hal.h>
#include <chprintf.h>

#include "sensors/proximity.h"
#include "sensors/VL53L0X/VL53L0X.h"

#define IR_SENSORNUM 0

static thread_t *TOFthd;
static thread_t *IRthd;

static THD_FUNCTION(TOFsensor_thd, arg) {
	(void) arg;
	chRegSetThreadName(__FUNCTION__);

	while (1) {
		volatile uint16_t testTOF = VL53L0X_get_dist_mm();
		chprintf((BaseSequentialStream *) &SD3, "TOF Distance: %d mm \r\n",
				testTOF);
		chThdSleepMilliseconds(100);
	}
}

static THD_FUNCTION(IRsensor_thd, arg) {
	(void) arg;
	chRegSetThreadName(__FUNCTION__);

//	calibrate_ir();
//	while (1) {
//		volatile int testIR = get_calibrated_prox(IR_SENSORNUM);
//		chprintf((BaseSequentialStream *) &SD3, "IR Level: %d \r\n", testIR);
//		chThdSleepMilliseconds(100);
//	}
}

void obstacle_detection_start(void) {

	proximity_start();
	VL53L0X_start();

	static THD_WORKING_AREA(TOFsensor_thd_wa, 1024);
	TOFthd = chThdCreateStatic(TOFsensor_thd_wa, sizeof(TOFsensor_thd_wa),
	NORMALPRIO, TOFsensor_thd, NULL);

	static THD_WORKING_AREA(IRsensor_thd_wa, 1024);
	IRthd = chThdCreateStatic(IRsensor_thd_wa, sizeof(IRsensor_thd_wa),
	NORMALPRIO, IRsensor_thd, NULL);
}

