/*
 * 	regulator.h
 *
 *	Course: Embedded Systems and Robotics, MICRO-315
 *  Created on: April 20, 2021
 *  Author: Robin Bonny (283196) and Andrea Bruder (283199)
 */

#ifndef REGULATOR_H_
#define REGULATOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "motor_control.h"

void lateral_regulator_start(void);
void frontal_regulator_start(void);
direction determine90(void);

#ifdef __cplusplus
}
#endif
#endif /* REGULATOR_H_ */
