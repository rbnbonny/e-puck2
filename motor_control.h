#ifndef MOTOR_CONTROL_H_
#define MOTOR_CONTROL_H_

#ifdef __cplusplus
extern "C" {
#endif

#define WHEEL_D 41 //mm
#define WHEEL_STEP 1000 //number of steps per rotation

typedef enum {
	LEFT = -1, STRAIGHT = 0, RIGHT = 1,
} direction;

void motor_turn(direction, uint16_t);
void motor_straight(void);
void motor_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* MOTOR_CONTROL_H_ */
