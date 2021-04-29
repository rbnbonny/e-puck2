#include <chprintf.h>
#include <obstacle_detection.h>
#include <mapping.h>
#include <motors.h>
#include <motor_control.h>
#include <math.h>

#define SQUARE_SIDE 124



static int32_t r_motor_pos_origin = 0;



int32_t mm_to_step(int dis, int tolerance){
	int32_t step = (dis- tolerance) * WHEEL_STEP/ (WHEEL_D * PI);
	return step;
}

void set_compass(uint8_t* compass, uint8_t dir){
	compass += dir;
	if(compass == 4)
		compass = 0;
	if(compass == -1)
		compass = 3;
}

void map_data(uint8_t compass, uint8_t compass_old, uint8_t* a, uint8_t* b){

	arr_map[a][b].TOF_dis = get_TOFIR_values().TOF_dist;
	arr_map[a][b].IR_r_pro = get_TOFIR_values().IR_r_prox;
	arr_map[a][b].IR_l_pro = get_TOFIR_values().IR_l_prox;
	arr_map[a][b].dir_old = compass_old;
	arr_map[a][b].dir = compass;

	switch(compass){
	case 0:
		a++;
		break;
	case 1:
		b++;
		break;
	case 2:
		a--;
		break;
	case 3:
		b--;
		break;
	}
}

void map_draw_walls(uint8_t i, uint8_t j){
	if(arr_map[i][j].IR_l_pro < 20){
		arr_map[i][j].draw &= 0x04;
	}
	if(arr_map[i][j].TOF_dis < 20){
		arr_map[i][j].draw &= 0x02;
	}
	if(arr_map[i][j].IR_r_pro < 20){
		arr_map[i][j].draw &= 0x01;
	}
}

void map_draw(void){
	for(uint8_t i = 0; i < 5; i++){
		for(uint8_t j = 0; j < 5; j++){
			switch(arr_map[i][j].dir_old){
			case NORTH:
				arr_map[i][j].draw &= 0x00;
				break;
			case EAST:
				arr_map[i][j].draw &= 0x08;
				break;
			case SOUTH:
				arr_map[i][j].draw &= 0x10;
				break;
			case WEST:
				arr_map[i][j].draw &= 0x18;
				break;
			}
			map_draw_walls(i, j);
		}
	}
}


static THD_WORKING_AREA(Mappig_Value_wa, 1024);
static THD_FUNCTION(Mapping_Value, arg) {

	(void) arg;
	chRegSetThreadName(__FUNCTION__);

	r_motor_pos_origin = right_motor_get_position(); //muss noch anderst initialisiert werden um Pos 0 bereits zu erkennen

	uint8_t a = 0; //y Koordinate
	uint8_t b = 0; //x Koordinate

	uint8_t compass_old = NORTH;
	uint8_t compass = NORTH;

	while (1) {

		uint8_t dir = 5;
		int32_t path = 0;

		path = right_motor_get_pos() - r_motor_pos_origin;

		if(path > mm_to_step(SQUARE_SIDE, 24)){
			dir = get_dir();
			if(dir == -1){
				map_data(set_compass(&compass, dir),compass_old, &a, &b);
				r_motor_pos_origin = right_motor_get_pos();
				compass_old = compass;
			}
			if(dir == 1){
				map_data(set_compass(dir), compass_old,  &a, &b);
				r_motor_pos_origin = right_motor_get_pos();
				compass_old = compass;
			}
			if(dir == 0){
				//mach nichts
			}
		}
		else if(path >= mm_to_step(SQUARE_SIDE, 0)){
			if(get_TOFIR_values().TOF_dist > 20){
				dir = 0; //soll geradeaus anziegen
				map_data(set_compass(dir), compass_old, &a, &b);
				r_motor_pos_origin = right_motor_get_pos();
				compass_old = compass;
			}
			if(get_TOFIR_values().TOF_dist <20){
				//mach nichts oder hol dir auch info in welche ritg du drehst
			}
		}
		else{
			//do nothing
		}





//		chThdSleepMilliseconds(IR_SAMPLING_WAIT);
	}
}


void process_image_start(void) {
	chThdCreateStatic(waMapping_Value, sizeof(waMapping_Value), NORMALPRIO,
			Mapping_Value, NULL);
}
