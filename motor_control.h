#ifndef MOTOR_CONTROL_H_
#define MOTOR_CONTROL_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	LEFT = 0, RIGHT,
} direction;

void motor_turn_90(direction);

#ifdef __cplusplus
}
#endif

#endif /* MOTOR_CONTROL_H_ */
