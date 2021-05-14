/*
 * 	mapping.h
 *
 *	Course: Embedded Systems and Robotics, MICRO-315
 *  Created on: April 27, 2021
 *  Author: Robin Bonny (283196) and Andrea Bruder (283199)
 */

#ifndef MAPPING_H_
#define MAPPING_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "motor_control.h"

typedef enum {
	NORTH = 0, EAST, SOUTH, WEST,
} galileo;

struct map {
	uint16_t TOF_dis;
	int IR_r_pro;
	int IR_l_pro;
	galileo dir_old;
	galileo dir;
	bool conquest;
};

void mapping_start(void);
uint8_t map_draw_f_wall(uint8_t, uint8_t);
uint8_t map_draw_l_wall(uint8_t, uint8_t);
uint8_t map_draw_r_wall(uint8_t, uint8_t);
void map_draw(uint8_t, uint8_t);
void set_turn(direction);

#ifdef __cplusplus
}
#endif
#endif /* MAPPING_H_ */

