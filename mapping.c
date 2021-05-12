#include <ch.h>
#include <hal.h>
#include <stdlib.h>
#include <chprintf.h>

#include <mapping.h>
#include <motors.h>
#include <blinker.h>
#include <music.h>
#include <motor_control.h>
#include <math.h>
#include <prox_sensors.h>

#define SQUARE_SIDE 124

static bool startFlag = true;

typedef enum {
	NOTHING = 0, ROBOT, WALL, EMPTY,
} picasso;

static int32_t r_motor_pos_origin = 0;
static int32_t l_motor_pos_origin = 0;
static direction turn_flag = 0;

static struct map arr_map[5][5] = { 0 };
static picasso general_map[11][11] = { 0 };

static uint8_t counter = 0;

int32_t mm_to_step(int dis, int tolerance) {
	int32_t step = (dis - tolerance) * WHEEL_STEP / (WHEEL_D * M_PI);
	return step;
}

void set_compass(galileo *compass, direction dir) {
	galileo comp = *compass;
	comp += dir;
	if (comp == 4)
		comp = 0;
	if (comp == 255)
		comp = 3;
	*compass = comp;
}

void map_data(galileo compass, galileo compass_old, uint8_t* a, uint8_t* b) {

	uint8_t i = *a;
	uint8_t j = *b;

	arr_map[i][j].TOF_dis = get_TOFIR_values().TOF_dist;
	arr_map[i][j].IR_r_pro = get_TOFIR_values().IR_r_prox;
	arr_map[i][j].IR_l_pro = get_TOFIR_values().IR_l_prox;
	arr_map[i][j].dir_old = compass_old;
	arr_map[i][j].dir = compass;
	arr_map[i][j].conquest = 1;

	map_draw(i, j);

	counter++;
	if (counter >= 25)
		counter = 0;

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

uint8_t map_draw_f_wall(uint8_t i, uint8_t j) {
	if (arr_map[i][j].TOF_dis < 70)
		return WALL;
	else
		return EMPTY;
}

uint8_t map_draw_l_wall(uint8_t i, uint8_t j) {
	if (arr_map[i][j].IR_l_pro > 150)
		return WALL;
	else
		return EMPTY;
}

uint8_t map_draw_r_wall(uint8_t i, uint8_t j) {
	if (arr_map[i][j].IR_r_pro > 150)
		return WALL;
	else
		return EMPTY;
}

void map_print(void) {

	for (int8_t i = 10; i >= 0; i--) {
		for (uint8_t j = 0; j < 11; j++) {
			switch (general_map[i][j]) {
			case NOTHING:
				chprintf((BaseSequentialStream *) &SD3, " ");
				break;
			case ROBOT:
				chprintf((BaseSequentialStream *) &SD3, ".");
				break;
			case WALL:
				chprintf((BaseSequentialStream *) &SD3, "#");
				break;
			case EMPTY:
				chprintf((BaseSequentialStream *) &SD3, ".");
				break;
			}
		}
		chprintf((BaseSequentialStream *) &SD3, "\r\n");
	}
	chprintf((BaseSequentialStream *) &SD3, "\r\n");

}

void map_draw(uint8_t i, uint8_t j) {

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

	map_print();
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

	r_motor_pos_origin = right_motor_get_pos() - mm_to_step(SQUARE_SIDE, 10); //muss noch anderst initialisiert werden um Pos 0 bereits zu erkennen
	l_motor_pos_origin = left_motor_get_pos() - mm_to_step(SQUARE_SIDE, 10); //muss noch anderst initialisiert werden um Pos 0 bereits zu erkennen

	uint8_t a = 0; //y Koordinate
	uint8_t b = 0; //x Koordinate

	static galileo compass_old = NORTH;
	static galileo compass = NORTH;

	int32_t path = 0;

	while (1) {

		path = ((right_motor_get_pos() - r_motor_pos_origin)
				+ (left_motor_get_pos() - l_motor_pos_origin)) / 2;

		switch (turn_flag) {
		case STRAIGHT:
			if (path > mm_to_step(SQUARE_SIDE, 0)
					&& get_TOFIR_values().TOF_dist > 50) {
				set_compass(&compass, turn_flag);
				map_data(compass, compass_old, &a, &b);
				r_motor_pos_origin = right_motor_get_pos();
				l_motor_pos_origin = left_motor_get_pos();
				compass_old = compass;
			}

			break;
		case LEFT:
			set_compass(&compass, turn_flag);
			map_data(compass, compass_old, &a, &b);
			r_motor_pos_origin = right_motor_get_pos();
			l_motor_pos_origin = left_motor_get_pos();
			compass_old = compass;
			turn_flag = STRAIGHT;
			break;
		case RIGHT:
			set_compass(&compass, turn_flag);
			map_data(compass, compass_old, &a, &b);
			r_motor_pos_origin = right_motor_get_pos();
			l_motor_pos_origin = left_motor_get_pos();
			compass_old = compass;
			turn_flag = STRAIGHT;
			break;
		default:
			break;
		}

		chThdSleepMilliseconds(500);
	}
}

void mapping_start(void) {
	chThdCreateStatic(Mapping_wa, sizeof(Mapping_wa),
	NORMALPRIO, Mapping, NULL);
}

void set_turn(direction dir) {
	turn_flag = dir;
}
