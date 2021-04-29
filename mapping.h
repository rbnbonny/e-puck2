

#ifndef MAPPING_H_
#define MAPPING_H_

#ifdef __cplusplus
extern "C" {
#endif

struct map{
	uint16_t TOF_dis;
	int IR_r_pro;
	int IR_l_pro;
	uint8_t dir_old;
	uint8_t dir;
	uint8_t draw;
};

struct map arr_map[5][5] = {0};


typedef enum {
	NORTH = 0, EAST, SOUTH, WEST,
} direction;


#ifdef __cplusplus
}
#endif
#endif /* MAPPING_H_ */

