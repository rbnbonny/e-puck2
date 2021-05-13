/*
 * 	prox_sensors.c
 *
 *	Course: Embedded Systems and Robotics, MICRO-315
 *  Created on: April 12, 2021
 *  Author: Robin Bonny (283196) and Andrea Bruder (283199)
 */

#include <ch.h>
#include <hal.h>
#include <stdlib.h>

#include <msgbus/messagebus.h>
#include <sensors/proximity.h>
#include <sensors/VL53L0X/VL53L0X.h>

#include "prox_sensors.h"

#define TOF_SAMPLING_WAIT 120
#define IR_SAMPLING_WAIT 80

static TOFIR_msg_t TOFIR_values;

messagebus_t bus;
MUTEX_DECL(bus_lock);
CONDVAR_DECL(bus_condvar);

static THD_WORKING_AREA(TOFsensor_wa, 1024);
static THD_FUNCTION(TOFsensor, arg) {
	(void) arg;
	chRegSetThreadName(__FUNCTION__);

	while (1) {
		TOFIR_values.TOF_dist = VL53L0X_get_dist_mm();
		chThdSleepMilliseconds(TOF_SAMPLING_WAIT);
	}
}

static THD_WORKING_AREA(IRsensor_wa, 1024);
static THD_FUNCTION(IRsensor, arg) {
	(void) arg;
	chRegSetThreadName(__FUNCTION__);

	messagebus_topic_t *proximity_topic = messagebus_find_topic_blocking(&bus,
			"/proximity");
	proximity_msg_t prox_values;

	while (1) {
		// Wait for data to be advertised on message bus
		messagebus_topic_wait(proximity_topic, &prox_values,
				sizeof(prox_values));
		TOFIR_values.IR_r_prox = get_prox(IR_SENSORNUM_R);
		TOFIR_values.IR_rf_prox = get_prox(IR_SENSORNUM_RF);
		TOFIR_values.IR_l_prox = get_prox(IR_SENSORNUM_L);
		TOFIR_values.IR_lf_prox = get_prox(IR_SENSORNUM_LF);

		chThdSleepMilliseconds(IR_SAMPLING_WAIT);
	}
}

void obstacle_detection_start(void) {

	messagebus_init(&bus, &bus_lock, &bus_condvar);

	// Start sensors
	VL53L0X_start();
	proximity_start();

	chThdCreateStatic(TOFsensor_wa, sizeof(TOFsensor_wa),
	NORMALPRIO, TOFsensor, NULL);

	chThdCreateStatic(IRsensor_wa, sizeof(IRsensor_wa),
	NORMALPRIO, IRsensor, NULL);
}

// Returns sensor data required in all other modules
TOFIR_msg_t get_TOFIR_values(void) {
	return TOFIR_values;
}
