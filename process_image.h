#ifndef PROCESS_IMAGE_H
#define PROCESS_IMAGE_H

void process_image_start(void);
void binary_image(uint8_t*);
void binary_correction(uint8_t*);
uint8_t edge_detection(uint8_t*);
uint8_t average_image(uint8_t*, uint16_t, uint16_t);
uint8_t get_barcode_number(void);


#endif /* PROCESS_IMAGE_H */
