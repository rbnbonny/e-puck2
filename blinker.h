#ifndef BLINKER_H_
#define BLINKER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <motor_control.h>

void blinker_start(void);
void call_blinker(direction);

#ifdef __cplusplus
}
#endif
#endif /* BLINKER_H_ */
