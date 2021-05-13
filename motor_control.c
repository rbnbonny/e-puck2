/*
 * 	motor_control.c
 *
 *	Course: Embedded Systems and Robotics, MICRO-315
 *  Created on: April 12, 2021
 *  Author: Robin Bonny (283196) and Andrea Bruder (283199)
 */

#include <ch.h>
#include <hal.h>
#include <stdlib.h>

#include <motors.h>

#include "motor_control.h"
#include "main.h"

#define FULL_ANGLE 360

/**
* @brief   				calculates the steps to turn out of the angle
*
* @param[in] angle		angle to turn (in degrees)
* @return wheel_steps	steps to drive until it has turned the desired angle
*/
uint16_t motor_turn_step(uint16_t angle) {

	float relative_turn = 0;
	uint16_t wheel_steps = 0;

	relative_turn = angle * WHEEL_STEP / FULL_ANGLE;
	wheel_steps = PUCK_D * (uint16_t) relative_turn / WHEEL_D;

	return wheel_steps;
}

/**
* @brief   				turns the robot towards left or right
*
* @param[in] angle		angle to turn
* @param[in] dir		towards which side the robot has to turn (left or right)
*/
void motor_turn(direction dir, uint16_t angle) {
	uint16_t wheel_steps = 0;
	int32_t pos_right = 0;
	int32_t pos_left = 0;

	pos_right = right_motor_get_pos();
	pos_left = left_motor_get_pos();

	wheel_steps = motor_turn_step(angle);

	if (dir == LEFT) {
		right_motor_set_speed(ROTSPEED);
		left_motor_set_speed(-ROTSPEED);
	} else if (dir == RIGHT) {
		right_motor_set_speed(-ROTSPEED);
		left_motor_set_speed(ROTSPEED);
	}

	while (1) {
		if (right_motor_get_pos() - pos_right > wheel_steps) {
			right_motor_set_speed(0);
			left_motor_set_speed(0);
			break;
		}
		if (left_motor_get_pos() - pos_left > wheel_steps) {
			right_motor_set_speed(0);
			left_motor_set_speed(0);
			break;
		}
	}
}

/**
* @brief   				robot moves straight forward
*/
void motor_straight(void) {
	left_motor_set_speed(MOTORSPEED);
	right_motor_set_speed(MOTORSPEED);
}

/**
* @brief   				robot stops driving
*/
void motor_stop(void) {
	left_motor_set_speed(0);
	right_motor_set_speed(0);
}
