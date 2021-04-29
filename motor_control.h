#ifndef MOTOR_CONTROL_H_
#define MOTOR_CONTROL_H_
#define MOTOR_SPEED 200

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	LEFT = -1, RIGHT = 1,
} direction;

void motor_turn(direction, uint16_t);
void motor_straight(void);
uint8_t get_dir(void);

#ifdef __cplusplus
}
#endif

#endif /* MOTOR_CONTROL_H_ */
