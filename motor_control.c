#include <ch.h>
#include <hal.h>
#include <stdlib.h>
#include <math.h>
#include <chprintf.h>

#include <main.h>
#include <motor_control.h>
#include <motors.h>

#define WHEEL_D 41 //mm
#define WHEEL_STEP 1000 //number of steps per rotation

uint16_t motor_turn_step(uint16_t angle) {

	float relative_turn = 0;
	uint16_t wheel_steps = 0;

	relative_turn = angle * WHEEL_STEP / 360;
	wheel_steps = PUCK_D * (uint16_t) relative_turn / WHEEL_D;

	return wheel_steps;
}

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
		//chprintf((BaseSequentialStream*)&SD3, "pos_diff = %d\r\n", right_motor_get_pos()-pos_right);
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

void motor_straight(void) {
	left_motor_set_speed(MOTORSPEED);
	right_motor_set_speed(MOTORSPEED);
}
