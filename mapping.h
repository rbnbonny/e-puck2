

#ifndef MAPPING_H_
#define MAPPING_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <motor_control.h>

struct map{
	uint16_t TOF_dis;
	int IR_r_pro;
	int IR_l_pro;
	uint8_t dir_old;
	uint8_t dir;
	uint8_t conquest;
};

typedef enum {
	NORTH = 0, EAST, SOUTH, WEST,
} galileo;

void mapping_start(void);
void set_turn(direction);

#ifdef __cplusplus
}
#endif
#endif /* MAPPING_H_ */

