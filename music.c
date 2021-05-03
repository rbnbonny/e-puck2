#include <ch.h>
#include <hal.h>
#include <stdlib.h>
#include <chprintf.h>

#include <music.h>
#include <process_image.h>
#include <audio/play_melody.h>
#include <audio/audio_thread.h>

static THD_WORKING_AREA(music_thd_wa, 8192);
static THD_FUNCTION(music_thd, arg) {
	(void) arg;
	chRegSetThreadName(__FUNCTION__);

	dac_start();
//	dac_power_speaker(true);
	playMelodyStart();

	while (1) {
		chprintf((BaseSequentialStream *) &SD3, "Code: %d \r\n",
				get_barcode_number());
		switch (get_barcode_number()) {
		case 1:
//			stopCurrentMelody();
			playMelody(MARIO_START, ML_SIMPLE_PLAY, NULL);
			break;
		case 2:
//			stopCurrentMelody();
			playMelody(MARIO_FLAG, ML_SIMPLE_PLAY, NULL);
			break;
		default:
			break;
		}
		chThdSleepMilliseconds(10);

//		playMelody(IMPOSSIBLE_MISSION, ML_SIMPLE_PLAY, NULL);
//		chThdSleepMilliseconds(10000);
//		stopCurrentMelody();
//		playMelody(WE_ARE_THE_CHAMPIONS, ML_SIMPLE_PLAY, NULL);
//		chThdSleepMilliseconds(10000);
//		playMelody(RUSSIA, ML_SIMPLE_PLAY, NULL);
//		waitMelodyHasFinished();
//		playMelody(UNDERWORLD, ML_SIMPLE_PLAY, NULL);
//		waitMelodyHasFinished();
//		playMelody(MARIO_START, ML_SIMPLE_PLAY, NULL);
//		waitMelodyHasFinished();
//		playMelody(MARIO_DEATH, ML_SIMPLE_PLAY, NULL);
//		waitMelodyHasFinished();
//		playMelody(MARIO_FLAG, ML_SIMPLE_PLAY, NULL);
//		waitMelodyHasFinished();
//		playMelody(WALKING, ML_SIMPLE_PLAY, NULL);
//		waitMelodyHasFinished();
//		playMelody(PIRATES_OF_THE_CARIBBEAN, ML_SIMPLE_PLAY, NULL);
//		waitMelodyHasFinished();
//		playMelody(SIMPSON, ML_SIMPLE_PLAY, NULL);
//		waitMelodyHasFinished();
//		playMelody(STARWARS, ML_SIMPLE_PLAY, NULL);
//		waitMelodyHasFinished();
//		playMelody(SANDSTORMS, ML_SIMPLE_PLAY, NULL);
//		waitMelodyHasFinished();
//		playMelody(SEVEN_NATION_ARMY, ML_SIMPLE_PLAY, NULL);
//		waitMelodyHasFinished();

	}
}

void music_start(void) {
	chThdCreateStatic(music_thd_wa, sizeof(music_thd_wa),
	NORMALPRIO, music_thd, NULL);
}
