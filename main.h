#ifndef MAIN_H
#define MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "camera/dcmi_camera.h"
#include "msgbus/messagebus.h"
#include "parameter/parameter.h"

#define CELLSIZE 120
#define PUCK_D 53 //mm

#define MOTORSPEED 1000
#define ROTSPEED 500
#define IMAGE_BUFFER_SIZE 640

/** Robot wide IPC bus. */
extern messagebus_t bus;

extern parameter_namespace_t parameter_root;

#ifdef __cplusplus
}
#endif

#endif
