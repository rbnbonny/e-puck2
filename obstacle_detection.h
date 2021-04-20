#ifndef OBSTACLE_DETECTION_H_
#define OBSTACLE_DETECTION_H_

#ifdef __cplusplus
extern "C" {
#endif

// IR Sensor IDs
#define IR_SENSORNUM_R 2
#define IR_SENSORNUM_RF 1
#define IR_SENSORNUM_L 5
#define IR_SENSORNUM_LF 6

// Struct containing proximity sensor date from TOF & IR
typedef struct {
	uint16_t TOF_dist;	// Frontal time-of-flight distance
	int IR_r_prox;		// Right IR3 proximity
	int IR_rf_prox;		// Right-front IR2 proximity
	int IR_l_prox;		// Left IR6 proximity
	int IR_lf_prox;		// Left-front IR7 proximity
} TOFIR_msg_t;

void obstacle_detection_start(void);
TOFIR_msg_t get_TOFIR_values(void);

#ifdef __cplusplus
}
#endif
#endif /* OBSTACLE_DETECTION_H_ */
