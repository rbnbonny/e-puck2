/*
 * 	process_image.h
 *
 *	Course: Embedded Systems and Robotics, MICRO-315
 *  Created on: April 12, 2021
 *  Author: Robin Bonny (283196) and Andrea Bruder (283199)
 */

#ifndef PROCESS_IMAGE_H
#define PROCESS_IMAGE_H

void process_image_start(void);
void binary_image(uint8_t*);
void binary_correction(uint8_t*);
uint8_t edge_detection(uint8_t*);
uint8_t get_barcode_number(void);


#endif /* PROCESS_IMAGE_H */
