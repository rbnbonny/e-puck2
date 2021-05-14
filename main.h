/*
 * 	main.h
 *
 *	Course: Embedded Systems and Robotics, MICRO-315
 *  Created on: April 08, 2021
 *  Author: Robin Bonny (283196) and Andrea Bruder (283199)
 */

#ifndef MAIN_H
#define MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "camera/dcmi_camera.h"
#include "msgbus/messagebus.h"
#include "parameter/parameter.h"

// Geometric constants
#define CELLSIZE 120
#define PUCK_D 53

#define MOTORSPEED 300
#define ROTSPEED 200

#define IMAGE_BUFFER_SIZE 640

/** Robot wide IPC bus. */
extern messagebus_t bus;

extern parameter_namespace_t parameter_root;

#ifdef __cplusplus
}
#endif

#endif /* MAIN_H_ */
