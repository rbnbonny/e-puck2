#ifndef MOTOR_CONTROL_H_
#define MOTOR_CONTROL_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	LEFT = 0, RIGHT,
} direction;

void motor_turn(direction, uint16_t);
void motor_straight(void);

#ifdef __cplusplus
}
#endif

#endif /* MOTOR_CONTROL_H_ */
