#include <ch.h>
#include <hal.h>
#include <stdlib.h>

#include <main.h>
#include <motor_control.h>
#include <motors.h>

void motor_turn_90(direction dir) {
	left_motor_set_pos(100);
	right_motor_set_pos(200);
}
