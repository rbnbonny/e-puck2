#ifndef MOTOR_CONTROL_H_
#define MOTOR_CONTROL_H_
#define MOTOR_SPEED 200

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	LEFT = 0, RIGHT,
} direction;

void motor_turn(direction, uint16_t);
void motor_straight(void);
uint8_t get_dir_flag(void);

#ifdef __cplusplus
}
#endif

#endif /* MOTOR_CONTROL_H_ */
