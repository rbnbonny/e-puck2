#include <ch.h>
#include <hal.h>
#include <chprintf.h>
#include <obstacle_detection.h>

#include "msgbus/messagebus.h"
#include "sensors/proximity.h"
#include "sensors/VL53L0X/VL53L0X.h"

#define IR_SENSORNUM_R 3
#define IR_SENSORNUM_RF 2
#define IR_SENSORNUM_L 6
#define IR_SENSORNUM_LF 7
#define TOF_SAMPLING_WAIT 100
//#define IR_SAMPLING_WAIT 100

//static thread_t *TOFthd;
//static thread_t *IRthd;

TOFIR_msg_t TOFIR_values;

messagebus_t bus;
MUTEX_DECL(bus_lock);
CONDVAR_DECL(bus_condvar);

static THD_WORKING_AREA(TOFsensor_thd_wa, 1024);
static THD_FUNCTION(TOFsensor_thd, arg) {
	(void) arg;
	chRegSetThreadName(__FUNCTION__);

	while (1) {
		TOFIR_values.TOF_dist = VL53L0X_get_dist_mm();
		chThdSleepMilliseconds(TOF_SAMPLING_WAIT);
	}
}

static THD_WORKING_AREA(IRsensor_thd_wa, 1024);
static THD_FUNCTION(IRsensor_thd, arg) {
	(void) arg;
	chRegSetThreadName(__FUNCTION__);

	messagebus_topic_t *proximity_topic = messagebus_find_topic_blocking(&bus,
			"/proximity");
	proximity_msg_t prox_values;

	calibrate_ir();
	while (1) {
		messagebus_topic_wait(proximity_topic, &prox_values,
				sizeof(prox_values));
		TOFIR_values.IR_r_prox = get_calibrated_prox(IR_SENSORNUM_R);
		TOFIR_values.IR_rf_prox = get_calibrated_prox(IR_SENSORNUM_RF);
		TOFIR_values.IR_l_prox = get_calibrated_prox(IR_SENSORNUM_L);
		TOFIR_values.IR_lf_prox = get_calibrated_prox(IR_SENSORNUM_LF);
//		chThdSleepMilliseconds(IR_SAMPLING_WAIT);
	}
}

static THD_WORKING_AREA(obstacle_detec_thd_wa, 2014);
static THD_FUNCTION(obstacle_detec_thd, arg) {
	(void) arg;
	chRegSetThreadName(__FUNCTION__);

	while (1) {
		chprintf((BaseSequentialStream *) &SD3, "TOF Distance: %d mm \r\n",
				TOFIR_values.TOF_dist);
		chprintf((BaseSequentialStream *) &SD3,
				"IR Levels: R%d RF%d L%d LF%d \r\n", TOFIR_values.IR_r_prox,
				TOFIR_values.IR_rf_prox, TOFIR_values.IR_l_prox,
				TOFIR_values.IR_lf_prox);
		chThdSleepMilliseconds(500);
	}

}

void obstacle_detection_start(void) {

	messagebus_init(&bus, &bus_lock, &bus_condvar);

	VL53L0X_start();
	proximity_start();

	chThdCreateStatic(TOFsensor_thd_wa, sizeof(TOFsensor_thd_wa),
	NORMALPRIO, TOFsensor_thd, NULL);

	chThdCreateStatic(IRsensor_thd_wa, sizeof(IRsensor_thd_wa),
	NORMALPRIO, IRsensor_thd, NULL);

	chThdCreateStatic(obstacle_detec_thd_wa, sizeof(obstacle_detec_thd_wa),
	NORMALPRIO, obstacle_detec_thd, NULL);
}

