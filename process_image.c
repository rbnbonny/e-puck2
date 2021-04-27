#include "ch.h"
#include "hal.h"
#include <chprintf.h>
#include <usbcfg.h>
#include <math.h>

#include <main.h>
#include <camera/po8030.h>

#include <process_image.h>

static BSEMAPHORE_DECL(image_ready_sem, TRUE);

uint8_t barcode_number = 0;

static THD_WORKING_AREA(waCaptureImage, 256);
static THD_FUNCTION(CaptureImage, arg) {

	chRegSetThreadName(__FUNCTION__);
	(void) arg;

	//Takes pixels 0 to IMAGE_BUFFER_SIZE of the line 10 + 11 (minimum 2 lines because reasons)
	po8030_advanced_config(FORMAT_RGB565, 0, 10, IMAGE_BUFFER_SIZE, 2,
			SUBSAMPLING_X1, SUBSAMPLING_X1);
	dcmi_enable_double_buffering();
	dcmi_set_capture_mode(CAPTURE_ONE_SHOT);
	dcmi_prepare();

	while (1) {

		//starts a capture
		dcmi_capture_start();
		//waits for the capture to be done
		wait_image_ready();
		//signals an image has been captured
		chBSemSignal(&image_ready_sem);
		//chprintf((BaseSequentialStream *)&SDU1, "time=%d \r\n", time2-time1);
	}
}

static THD_WORKING_AREA(waProcessImage, 1024);
static THD_FUNCTION(ProcessImage, arg) {

	chRegSetThreadName(__FUNCTION__);
	(void) arg;

	uint8_t *img_buff_ptr;
	uint8_t image[IMAGE_BUFFER_SIZE] = { 0 };

	while (1) {
		//waits until an image has been captured
		chBSemWait(&image_ready_sem);
		//gets the pointer to the array filled with the last image in RGB565    
		img_buff_ptr = dcmi_get_last_image_ptr();
		uint8_t a, b;
		for (uint16_t i = 0; i < IMAGE_BUFFER_SIZE; i++) {
			a = 0;
			b = 0;
			a = *img_buff_ptr; //high byte
			img_buff_ptr++;
			b = *img_buff_ptr; //low byte
			img_buff_ptr++;
			image[i] = ((a & 0x07) << 3) | ((b & 0xE0) >> 5);
		}

		binary_image(image);
		barcode_number = edge_detection(image);
//		chprintf((BaseSequentialStream *) &SDU1, "barcode = %d\r\n",
//				barcode_number);
		//SendUint8ToComputer(image, IMAGE_BUFFER_SIZE);

	}
}

void binary_image(uint8_t* image) {

	uint16_t mean = 0;
	for (uint16_t i = 0; i < IMAGE_BUFFER_SIZE; i++) {
		mean += image[i];
	}
	mean = mean / IMAGE_BUFFER_SIZE;

	static uint8_t image_prev[IMAGE_BUFFER_SIZE] = { 0 };
	static uint8_t image_buf[IMAGE_BUFFER_SIZE] = { 0 };

	for (uint16_t i = 0; i < IMAGE_BUFFER_SIZE; i++) {
		image_buf[i] = image[i];
		image[i] = sqrt(image[i] * image[i] + image_prev[i] * image_prev[i]);
		image_prev[i] = image_buf[i];
	}

	// Binning using mean intensity of image
	for (uint16_t i = 0; i < IMAGE_BUFFER_SIZE; i++) {
		if (image[i] < mean)
			image[i] = 0;
		else
			image[i] = 1;
	}
	binary_correction(image);
}

void binary_correction(uint8_t* image) {

	uint16_t j = 0;

	while (!image[j + 1] || !image[j + 2] || !image[j + 3] || !image[j + 4]
			|| !image[j + 5]) {
		image[j] = 1;
		j++;
	}

	j = 639;

	while (!image[j - 5] || !image[j - 4] || !image[j - 3] || !image[j - 2]
			|| !image[j - 1]) {
		image[j] = 1;
		j--;
	}

	for (uint16_t i = 1; i < IMAGE_BUFFER_SIZE - 2; i++) {
		if (!image[i]) {
			if (image[i - 1] && image[i + 1])
				image[i] = 1;
			if (image[i - 1] && !image[i + 1]) {
				if (image[i + 2])
					image[i] = 1;
				else
					image[i] = 0;
			}
		}
	}

	for (uint16_t i = IMAGE_BUFFER_SIZE - 2; i > 2; i--) {
		if (!image[i]) {
			if (image[i + 1] && image[i - 1])
				image[i] = 1;
			if (image[i + 1] && !image[i - 1]) {
				if (image[i - 2])
					image[i] = 1;
				else
					image[i] = 0;
			}
		}
	}
}

uint8_t edge_detection(uint8_t *image) {
	uint16_t edge_array[14] = { 0 };
	int16_t line_width[7] = { 0 };
	uint8_t b = 0;
	uint8_t a = 0;
	for (uint16_t i = 5; i < IMAGE_BUFFER_SIZE - 5; i++) {
		if ((image[i] - image[i + 1]) == 1) {
			edge_array[2 * a] = i;
		}
		if ((image[i] - image[i - 1]) == 1) {
			edge_array[2 * a + 1] = i + 1;
			a++;
			if (a == 7)
				break;
		}
	}

	for (a = 0; a < 7; a++) {
		line_width[a] = edge_array[2 * a + 1] - edge_array[2 * a];
		if (line_width[a] <= 0)
			return b = 0;;
	}

	for (a = 1; a < 5; a++) {
		if (line_width[a] > (line_width[0] + line_width[5]) / 2)
			line_width[a] = 1;
		else
			line_width[a] = 0;
	}

//	for(a = 1; a < 5; a++){
//		if(0.75*line_width[0] < line_width[a] && line_width[a] < 1.25*line_width[0])
//			line_width[a] = 1;
//		if(0.5*line_width[5] < line_width[a] && line_width[a] <= 2*line_width[5])
//			line_width[a] = 0;
//		if(line_width[a] != 0 || line_width[a] != 1)
//			b=0;
//		}
//	for(a = 1; a < 5; a++){
//		chprintf((BaseSequentialStream *)&SDU1, "%d = %d\r\n", a, line_width[a]);
//		chThdSleepMilliseconds(500);
//	}

	b = line_width[1] * 8 + line_width[2] * 4 + line_width[3] * 2
			+ line_width[4];
	return b;
}

void process_image_start(void) {
	chThdCreateStatic(waProcessImage, sizeof(waProcessImage), NORMALPRIO,
			ProcessImage, NULL);
	chThdCreateStatic(waCaptureImage, sizeof(waCaptureImage), NORMALPRIO,
			CaptureImage, NULL);
}

uint8_t get_barcode_number(void){
	return barcode_number;
}
