/*
 * 	process_image.c
 *
 *	Course: Embedded Systems and Robotics, MICRO-315
 *  Created on: April 12, 2021
 *  Author: Robin Bonny (283196) and Andrea Bruder (283199)
 */

#include "ch.h"
#include "hal.h"
#include <chprintf.h>
#include <usbcfg.h>
#include <math.h>

#include <main.h>
#include <camera/po8030.h>

#include <process_image.h>

#define PROCESSIMAGE_WAIT 50

static BSEMAPHORE_DECL(image_ready_sem, TRUE);

uint8_t barcode_number = 0;

static THD_WORKING_AREA(CaptureImage_wa, 256);
static THD_FUNCTION(CaptureImage, arg) {

	chRegSetThreadName(__FUNCTION__);
	(void) arg;

	//Takes pixels 0 to IMAGE_BUFFER_SIZE of the line 10 + 11 (minimum 2 lines because reasons)
	po8030_advanced_config(FORMAT_RGB565, 0, 100, IMAGE_BUFFER_SIZE, 2,
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
	}
}

static THD_WORKING_AREA(ProcessImage_wa, 1024);
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
		//transfers the green image bits into the image array
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

		//converts the image into a binary image
		binary_image(image);
		//converts the image into a decimal number
		barcode_number = edge_detection(image);
		chThdSleepMilliseconds(PROCESSIMAGE_WAIT);
	}
}


/**
* @brief   			converts the image into a binary image
*
* @param[in] image		array containing one line of the captured image
*/
void binary_image(uint8_t* image) {

	//takes the mean over all image points
	uint16_t mean = 0;
	for (uint16_t i = 0; i < IMAGE_BUFFER_SIZE; i++) {
		mean += image[i];
	}
	mean = mean / IMAGE_BUFFER_SIZE;

	static uint8_t image_prev[IMAGE_BUFFER_SIZE] = { 0 };
	static uint8_t image_buf[IMAGE_BUFFER_SIZE] = { 0 };

	//takes the root mean square value of the current and previous image to avoid fluctuations
	for (uint16_t i = 0; i < IMAGE_BUFFER_SIZE; i++) {
		image_buf[i] = image[i];
		image[i] = sqrt(image[i] * image[i] + image_prev[i] * image_prev[i]);
		image_prev[i] = image_buf[i];
	}

	//binning using mean intensity of image
	for (uint16_t i = 0; i < IMAGE_BUFFER_SIZE; i++) {
		if (image[i] < mean)
			image[i] = 0;
		else
			image[i] = 1;
	}

	//corrects vignetting effects
	binary_correction(image);
}

/**
* @brief   			corrects vignetting effects
*
* @param[in] image		array containing one line of the captured image in binary form
*/
void binary_correction(uint8_t* image) {

	/* goes from the left border towards the middle and checks if the next 5 consecutive points are 1.
	* if they aren't, it puts the current point to 1 and corrects the vignetting effect.
	*/
	uint16_t j = 0;
	while (!image[j + 1] || !image[j + 2] || !image[j + 3] || !image[j + 4]
			|| !image[j + 5]) {
		image[j] = 1;
		j++;
	}

	/* goes from the right border towards the middle and checks i the next 5 consecutive points are 1.
	 * if they aren't, it puts the current point to 1 and corrects the vignetting effect.
	 */
	j = 639;
	while (!image[j - 5] || !image[j - 4] || !image[j - 3] || !image[j - 2]
			|| !image[j - 1]) {
		image[j] = 1;
		j--;
	}

	//program takes an image point and compares it with maximal two neighbor points on each side to avoid fluctuations at the barcode edges.
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

/**
* @brief   			converts the image into a binary image
*
* @param[in] image		array containing one line of the captured image
* @return    b			decimal number of the barcode
*/
uint8_t edge_detection(uint8_t *image) {

	//detects the edges of the barcode
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

	//checks if all bars have a positive length
	for (a = 0; a < 7; a++) {
		line_width[a] = edge_array[2 * a + 1] - edge_array[2 * a];
		if (line_width[a] <= 0)
			return b = 0;;
	}

	//compares bars 2 to 5 with bars 1 and 6
	for (a = 1; a < 5; a++) {
		if (line_width[a] > (line_width[0] + line_width[5]) / 2)
			line_width[a] = 1;
		else
			line_width[a] = 0;
	}

	//converts binary number into decimal number
	b = line_width[1] * 8 + line_width[2] * 4 + line_width[3] * 2
			+ line_width[4];
	return b;
}

void process_image_start(void) {
	chThdCreateStatic(ProcessImage_wa, sizeof(ProcessImage_wa), NORMALPRIO,
			ProcessImage, NULL);
	chThdCreateStatic(CaptureImage_wa, sizeof(CaptureImage_wa), NORMALPRIO,
			CaptureImage, NULL);
}

uint8_t get_barcode_number(void) {
	return barcode_number;
}
