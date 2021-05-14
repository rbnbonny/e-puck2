/*
 * 	mapping.c
 *
 *	Course: Embedded Systems and Robotics, MICRO-315
 *  Created on: April 27, 2021
 *  Author: Robin Bonny (283196) and Andrea Bruder (283199)
 */

#include <ch.h>
#include <hal.h>
#include <stdlib.h>
#include <math.h>
#include <chprintf.h>

#include <motors.h>

#include "mapping.h"
#include "prox_sensors.h"
#include "blinker.h"
#include "music.h"

//length of a square in our laser-cut environment
#define SQUARE_SIDE 124

// Dimensions of the labyrinth i.e. 5x5
#define CELLS 5

#define SIDEMAP_THRESHOLD	150
#define FRONTMAP_THRESHOLD	70
#define FRONTWALL_THRESHOLD	50
#define MAPPING_WAIT		500

// Symbols to print
#define NOTHING_CHAR	" "
#define ROBOT_CHAR		"."
#define WALL_CHAR		"#"
#define EMPTY_CHAR		"."

// Elements to draw map
typedef enum {
	NOTHING = 0, ROBOT, WALL, EMPTY,
} picasso;

static bool startFlag = true;
static int32_t r_motor_pos_origin = 0;
static int32_t l_motor_pos_origin = 0;
static direction turnFlag = 0;

static struct map arr_map[CELLS][CELLS] = { 0 };
static picasso general_map[2 * CELLS + 1][2 * CELLS + 1] = { 0 };

/**
 * @brief   				converts a mm-input into steps to drive
 *
 * @param[in] dis			1. value to set the mm distance
 * @param[in] tolerance		2. value to set the mm distance
 * @return step				steps to drive ouput
 */
int32_t mm_to_step(int dis, int tolerance) {
	int32_t step = (dis - tolerance) * WHEEL_STEP / (WHEEL_D * M_PI);
	return step;
}

/**
 * @brief   				sets the new orientation to the robot
 *
 * @param[in] compass		orientation of the robot
 * @param[in] dir			orientation change
 */
void set_compass(galileo *compass, direction dir) {
	galileo comp = *compass;
	comp += dir;
	if (comp == 4)
		comp = 0;
	if (comp == 255)
		comp = 3;
	*compass = comp;
}

/**
 * @brief   				captures the environment at a specific point to draw later the map
 *
 * @param[in] compass		next orientation of the robot after the capture
 * @param[in] compass_old 	current orientation of the robot
 * @param[in] a				y axis coordinate for the map
 * @param[in] b 			x axis coordinate for the map
 */
void map_data(galileo compass, galileo compass_old, uint8_t* a, uint8_t* b) {

	uint8_t i = *a;
	uint8_t j = *b;

	//stores front / lateral distances, current and future orientation if map field has been updated
	arr_map[i][j].TOF_dis = get_TOFIR_values().TOF_dist;
	arr_map[i][j].IR_r_pro = get_TOFIR_values().IR_r_prox;
	arr_map[i][j].IR_l_pro = get_TOFIR_values().IR_l_prox;
	arr_map[i][j].dir_old = compass_old;
	arr_map[i][j].dir = compass;
	arr_map[i][j].conquest = true;

	//draws the map
	map_draw(i, j);

	//sets up next map field, based on the future orientation
	switch (compass) {
	case NORTH:
		i++;
		break;
	case EAST:
		j++;
		break;
	case SOUTH:
		i--;
		break;
	case WEST:
		j--;
		break;
	}
	*a = i;
	*b = j;
}

//returns wall or empty in front of the robot based on the ToF value
uint8_t map_draw_f_wall(uint8_t i, uint8_t j) {
	//if TOF value is smaller than FRONTMAP_THRESHOLD, there's a wall in front of the robot
	if (arr_map[i][j].TOF_dis < FRONTMAP_THRESHOLD)
		return WALL;
	else
		return EMPTY;
}

//returns wall or empty left to the robot
uint8_t map_draw_l_wall(uint8_t i, uint8_t j) {
	//if IR value is larger than SIDEMAP_THRESHOLD, there's a lateral wall
	if (arr_map[i][j].IR_l_pro > SIDEMAP_THRESHOLD)
		return WALL;
	else
		return EMPTY;
}

//returns wall or empty right to the robot
uint8_t map_draw_r_wall(uint8_t i, uint8_t j) {
	//if IR value is larger than SIDEMAP_THRESHOLD, there's a lateral wall
	if (arr_map[i][j].IR_r_pro > SIDEMAP_THRESHOLD)
		return WALL;
	else
		return EMPTY;
}

//sends the map to the computer
void map_print(void) {

	for (int8_t i = 2 * CELLS; i >= 0; i--) {
		for (uint8_t j = 0; j < (2 * CELLS + 1); j++) {
			switch (general_map[i][j]) {
			case NOTHING:
				chprintf((BaseSequentialStream *) &SD3, NOTHING_CHAR);
				break;
			case ROBOT:
				chprintf((BaseSequentialStream *) &SD3, ROBOT_CHAR);
				break;
			case WALL:
				chprintf((BaseSequentialStream *) &SD3, WALL_CHAR);
				break;
			case EMPTY:
				chprintf((BaseSequentialStream *) &SD3, EMPTY_CHAR);
				break;
			}
		}
		chprintf((BaseSequentialStream *) &SD3, "\r\n");
	}
	chprintf((BaseSequentialStream *) &SD3, "\r\n");

}

/**
 * @brief   				draws the map
 *
 * @param[in] i				y axis coordinate
 * @param[in] j				x axis coordinate
 */
void map_draw(uint8_t i, uint8_t j) {

	/*maps the 5x5 field into a 11x11 field. odd numbers are robot positions, even numbers are "wall" positions in the map array.
	 * As the robot can not see "border fields" (for example position [2][2] in the map field (start counting with 0)
	 * this function also writes the seen values to the neighbor points. (for example if the robot is in field [7][7] direction north and
	 * it sees a border at [7][8] it draws the border as well to [6][8] and [8][8]
	 */
	general_map[2 * i + 1][2 * j + 1] = ROBOT;
	switch (arr_map[i][j].dir_old) {  //conversion is 2*i + 1
	case NORTH:
		for (uint8_t k = 2 * j; k < 2 * j + 3; k++) {
			general_map[2 * i + 2][k] = map_draw_f_wall(i, j); //2*j+1
		}
		for (uint8_t k = 2 * i; k < 2 * i + 3; k++) {
			general_map[k][2 * j] = map_draw_l_wall(i, j); //2*i+1
			general_map[k][2 * j + 2] = map_draw_r_wall(i, j); //2*i+1
		}
		break;
	case EAST:
		for (uint8_t k = 2 * i; k < 2 * i + 3; k++) {
			general_map[k][2 * j + 2] = map_draw_f_wall(i, j);
		}
		for (uint8_t k = 2 * j; k < 2 * j + 3; k++) {
			general_map[2 * i + 2][k] = map_draw_l_wall(i, j);
			general_map[2 * i][k] = map_draw_r_wall(i, j);
		}
		break;
	case SOUTH:
		for (uint8_t k = 2 * j; k < 2 * j + 3; k++) {
			general_map[2 * i][k] = map_draw_f_wall(i, j);
		}
		for (uint8_t k = 2 * i; k < 2 * i + 3; k++) {
			general_map[k][2 * j + 2] = map_draw_l_wall(i, j);
			general_map[k][2 * j] = map_draw_r_wall(i, j);
		}
		break;
	case WEST:
		for (uint8_t k = 2 * i; k < 2 * i + 3; k++) {
			general_map[k][2 * j] = map_draw_f_wall(i, j);
		}
		for (uint8_t k = 2 * j; k < 2 * j + 3; k++) {
			general_map[2 * i][k] = map_draw_l_wall(i, j);
			general_map[2 * i + 2][k] = map_draw_r_wall(i, j);
		}
		break;
	}

	//sends the map to the computer
	map_print();
	//makes party when the robot has finished one cycle and is back on position (0;0)
	if (i == 0 && j == 0 && startFlag == false) {
		party_blinker();
		party_music();
	} else {
		startFlag = false;
	}
}

static THD_WORKING_AREA(Mapping_wa, 4096);
static THD_FUNCTION(Mapping, arg) {

	(void) arg;
	chRegSetThreadName(__FUNCTION__);

	r_motor_pos_origin = right_motor_get_pos() - mm_to_step(SQUARE_SIDE, 10); // we begin 10mm in front of the first mapping point
	l_motor_pos_origin = left_motor_get_pos() - mm_to_step(SQUARE_SIDE, 10);

	//robot starts its cycle at the bottom left corner oriented towards north
	uint8_t a = 0; //y coordinate
	uint8_t b = 0; //x coordinate

	static galileo compass_old = NORTH;
	static galileo compass = NORTH;

	int32_t path = 0;

	while (1) {

		path = ((right_motor_get_pos() - r_motor_pos_origin)
				+ (left_motor_get_pos() - l_motor_pos_origin)) / 2;

		switch (turnFlag) {
		case STRAIGHT:
			//if TOF larger than this value, there's no frontal wall.
			if (path
					> mm_to_step(SQUARE_SIDE,
							0) && get_TOFIR_values().TOF_dist > FRONTWALL_THRESHOLD) {
				set_compass(&compass, turnFlag);
				map_data(compass, compass_old, &a, &b);
				r_motor_pos_origin = right_motor_get_pos();
				l_motor_pos_origin = left_motor_get_pos();
				compass_old = compass;
			}
			break;
		case LEFT:
		case RIGHT:
			set_compass(&compass, turnFlag);
			map_data(compass, compass_old, &a, &b);
			r_motor_pos_origin = right_motor_get_pos();
			l_motor_pos_origin = left_motor_get_pos();
			compass_old = compass;
			turnFlag = STRAIGHT;
			break;
		default:
			break;
		}
		chThdSleepMilliseconds(MAPPING_WAIT);
	}
}

void mapping_start(void) {
	chThdCreateStatic(Mapping_wa, sizeof(Mapping_wa),
	NORMALPRIO, Mapping, NULL);
}

void set_turn(direction dir) {
	turnFlag = dir;
}
