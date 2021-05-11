#include <ch.h>
#include <hal.h>
#include <stdlib.h>
#include <chprintf.h>

#include <music.h>
#include <process_image.h>
#include <obstacle_detection.h>
#include <motor_control.h>
#include <blinker.h>
#include <audio/play_melody.h>
#include <audio/audio_thread.h>

static bool partyMusic = false;

static THD_WORKING_AREA(music_thd_wa, 8192);
static THD_FUNCTION(music_thd, arg) {
	(void) arg;
	chRegSetThreadName(__FUNCTION__);

	static uint8_t confirm = 0;

	dac_start();
	playMelodyStart();

	uint8_t barcode_num;
	static uint8_t last_code = 0;

	while (1) {
		barcode_num = get_barcode_number();
		if (barcode_num > 0 && get_TOFIR_values().TOF_dist < 140 && confirm < 3
				&& barcode_num != last_code) {
			confirm++;
		} else if (confirm >= 3 && barcode_num > 0) {
//			chprintf((BaseSequentialStream *) &SD3, "Code: %d \r\n",
//					get_barcode_number());

			call_blinker(STRAIGHT, barcode_num);
//			switch (barcode_num) {
//			case 1:
//				playMelody(IMPOSSIBLE_MISSION, ML_FORCE_CHANGE, NULL);
//				break;
//			case 2:
////				playMelody(RUSSIA, ML_FORCE_CHANGE, NULL);
//				playMelody(MARIO, ML_FORCE_CHANGE, NULL);
//				break;
//			case 3:
//				playMelody(WE_ARE_THE_CHAMPIONS, ML_FORCE_CHANGE, NULL);
//				break;
//			case 4:
////				playMelody(MARIO, ML_FORCE_CHANGE, NULL);
//				playMelody(IMPOSSIBLE_MISSION, ML_FORCE_CHANGE, NULL);
//				break;
//			case 5:
//				playMelody(UNDERWORLD, ML_FORCE_CHANGE, NULL);
//				break;
//			case 6:
////				playMelody(WALKING, ML_FORCE_CHANGE, NULL);
//				playMelody(STARWARS, ML_FORCE_CHANGE, NULL);
//				break;
//			case 7:
//				playMelody(PIRATES_OF_THE_CARIBBEAN, ML_FORCE_CHANGE, NULL);
//				break;
//			case 8:
//				playMelody(SIMPSON, ML_FORCE_CHANGE, NULL);
//				break;
//			case 9:
//				playMelody(STARWARS, ML_FORCE_CHANGE, NULL);
//				break;
//			default:
//				break;
//			}
			last_code = barcode_num;
			confirm = 0;
		}
		if (partyMusic) {
			playMelody(MARIO_FLAG, ML_FORCE_CHANGE, NULL);
			partyMusic = false;
		}
		chThdSleepMilliseconds(200);
	}
}

void music_start(void) {
	chThdCreateStatic(music_thd_wa, sizeof(music_thd_wa),
	NORMALPRIO, music_thd, NULL);
}

void party_music(void) {
	partyMusic = true;
}
