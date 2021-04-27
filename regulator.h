#ifndef REGULATOR_H_
#define REGULATOR_H_

#ifdef __cplusplus
extern "C" {
#endif

void lateral_regulator_start(void);
void frontal_regulator_start(void);
void frontal_obstacle_wait(void);

#ifdef __cplusplus
}
#endif
#endif /* REGULATOR_H_ */
