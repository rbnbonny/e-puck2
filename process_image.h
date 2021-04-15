#ifndef PROCESS_IMAGE_H
#define PROCESS_IMAGE_H

float get_distance_cm(void);
void process_image_start(void);
void edge_detection(uint8_t*);
uint8_t average_image(uint8_t*, uint16_t, uint16_t);

#endif /* PROCESS_IMAGE_H */
