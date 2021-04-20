#include <ch.h>
#include <hal.h>
#include <chprintf.h>

#include <regulator.h>
#include <main.h>
#include <obstacle_detection.h>
#include <motors.h>


static THD_WORKING_AREA(regulator_thd_wa, 1024);
static THD_FUNCTION(regulator_thd, arg) {
	(void) arg;
	chRegSetThreadName(__FUNCTION__);

	while(1){

	}

}

void regulator_start(void) {
	chThdCreateStatic(regulator_thd_wa, sizeof(regulator_thd_wa),
	NORMALPRIO, regulator_thd, NULL);
}
