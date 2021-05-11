#include <ch.h>
#include <hal.h>
#include <chprintf.h>
#include <obstacle_detection.h>

#include "msgbus/messagebus.h"
#include "sensors/proximity.h"
#include "sensors/VL53L0X/VL53L0X.h"

#define TOF_SAMPLING_WAIT 120
#define IR_SAMPLING_WAIT 80

static TOFIR_msg_t TOFIR_values;

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

	while (1) {
		messagebus_topic_wait(proximity_topic, &prox_values,
				sizeof(prox_values));
		TOFIR_values.IR_r_prox = get_prox(IR_SENSORNUM_R);
		TOFIR_values.IR_rf_prox = get_prox(IR_SENSORNUM_RF);
		TOFIR_values.IR_l_prox = get_prox(IR_SENSORNUM_L);
		TOFIR_values.IR_lf_prox = get_prox(IR_SENSORNUM_LF);
//		chprintf((BaseSequentialStream *) &SD3, "TOF Distance: %d mm \r\n",
//				TOFIR_values.TOF_dist);
//		chprintf((BaseSequentialStream *) &SD3,
//				"IR Levels: R%d RF%d L%d LF%d \r\n", TOFIR_values.IR_r_prox,
//				TOFIR_values.IR_rf_prox, TOFIR_values.IR_l_prox,
//				TOFIR_values.IR_lf_prox);
		chThdSleepMilliseconds(IR_SAMPLING_WAIT);
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
}

TOFIR_msg_t get_TOFIR_values(void) {
	return TOFIR_values;
}
