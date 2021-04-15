#include "ch.h"
#include "hal.h"
#include <chprintf.h>
#include <usbcfg.h>

#include <main.h>
#include <camera/po8030.h>

#include <process_image.h>


static float distance_cm = 0;

//semaphore
static BSEMAPHORE_DECL(image_ready_sem, TRUE);

static THD_WORKING_AREA(waCaptureImage, 256);
static THD_FUNCTION(CaptureImage, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

	systime_t time1 = chVTGetSystemTime();
	systime_t time2;

	//Takes pixels 0 to IMAGE_BUFFER_SIZE of the line 10 + 11 (minimum 2 lines because reasons)
	po8030_advanced_config(FORMAT_RGB565, 0, 10, IMAGE_BUFFER_SIZE, 2, SUBSAMPLING_X1, SUBSAMPLING_X1);
	dcmi_enable_double_buffering();
	dcmi_set_capture_mode(CAPTURE_ONE_SHOT);
	dcmi_prepare();

    while(1){

    	time2 = chVTGetSystemTime();
        //starts a capture
		dcmi_capture_start();
		//waits for the capture to be done
		wait_image_ready();
		//signals an image has been captured
		chBSemSignal(&image_ready_sem);
		//chprintf((BaseSequentialStream *)&SDU1, "time=%d \r\n", time2-time1);
		time1 = time2;
    }
}


static THD_WORKING_AREA(waProcessImage, 1024);
static THD_FUNCTION(ProcessImage, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

	uint8_t *img_buff_ptr;
	uint8_t image[IMAGE_BUFFER_SIZE] = {0};

    while(1){
    	//waits until an image has been captured
        chBSemWait(&image_ready_sem);
		//gets the pointer to the array filled with the last image in RGB565    
		img_buff_ptr = dcmi_get_last_image_ptr();
		uint8_t a, b;
		for(uint16_t i = 0; i < IMAGE_BUFFER_SIZE; i++){
			a = 0;
			b = 0;
			a = *img_buff_ptr; //high byte
			img_buff_ptr++;
			b = *img_buff_ptr; //low byte
			img_buff_ptr++;
			image[i] = ((a & 0x07) << 3) | ((b & 0xE0 ) >> 5);
		}

		edge_detection(image);
		SendUint8ToComputer(image, IMAGE_BUFFER_SIZE);
    }
}



void edge_detection(uint8_t *image){
	uint16_t edge_left = 0, edge_right = 639;
	int16_t new_difference = 0;
	int16_t	old_difference = 0;
	float width = 0;
	for(uint16_t i = 0; i < IMAGE_BUFFER_SIZE-2; i++){
		new_difference = image[i]- image[i+2];
		if(new_difference > old_difference){
			old_difference = new_difference;
			edge_left = i;
		}
	}

	new_difference = 0;
	old_difference = 0;
	for(uint16_t i = 0; i < IMAGE_BUFFER_SIZE-2; i++){
		new_difference = image[i+2] - image[i];
		if(new_difference > old_difference){
			old_difference = new_difference;
			edge_right = i+2;
		}
	}

	//width
	width = edge_right - edge_left;
	distance_cm = 1300/ width; //trial and error
}


float get_distance_cm(void){
	return distance_cm;
}

void process_image_start(void){
	chThdCreateStatic(waProcessImage, sizeof(waProcessImage), NORMALPRIO, ProcessImage, NULL);
	chThdCreateStatic(waCaptureImage, sizeof(waCaptureImage), NORMALPRIO, CaptureImage, NULL);
}
