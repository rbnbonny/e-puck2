#include <ch.h>
#include <hal.h>
#include <stdlib.h>
#include <chprintf.h>

#include <music.h>
#include <process_image.h>
#include <obstacle_detection.h>
#include <audio/play_melody.h>
#include <audio/audio_thread.h>

static THD_WORKING_AREA(music_thd_wa, 8192);
static THD_FUNCTION(music_thd, arg) {
	(void) arg;
	chRegSetThreadName(__FUNCTION__);

	static bool confirmFlag = false;

	dac_start();
//	dac_power_speaker(true);
	playMelodyStart();

	while (1) {
//		chprintf((BaseSequentialStream *) &SD3, "Code: %d \r\n",
//				get_barcode_number());

		if (get_barcode_number() > 0 && get_TOFIR_values().TOF_dist < 80) {
			confirmFlag = true;
		} else if (confirmFlag) {

			switch (get_barcode_number()) {
			case 1:
				playMelody(IMPOSSIBLE_MISSION, ML_FORCE_CHANGE, NULL);
				break;
			case 2:
				playMelody(WE_ARE_THE_CHAMPIONS, ML_FORCE_CHANGE, NULL);
				break;
			case 3:
				playMelody(RUSSIA, ML_FORCE_CHANGE, NULL);
				break;
			case 4:
				playMelody(MARIO, ML_FORCE_CHANGE, NULL);
				break;
			case 5:
				playMelody(UNDERWORLD, ML_FORCE_CHANGE, NULL);
				break;
			case 6:
				playMelody(WALKING, ML_FORCE_CHANGE, NULL);
				break;
			case 7:
				playMelody(PIRATES_OF_THE_CARIBBEAN, ML_FORCE_CHANGE, NULL);
				break;
			case 8:
				playMelody(SIMPSON, ML_FORCE_CHANGE, NULL);
				break;
			case 9:
				playMelody(STARWARS, ML_FORCE_CHANGE, NULL);
				break;
			default:
				break;
				confirmFlag = false;
			}
		}
		chThdSleepMilliseconds(100);
	}
}

void music_start(void) {
	chThdCreateStatic(music_thd_wa, sizeof(music_thd_wa),
	NORMALPRIO, music_thd, NULL);
}
