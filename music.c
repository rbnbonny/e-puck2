#include <ch.h>
#include <hal.h>
#include <stdlib.h>
#include <chprintf.h>

#include <music.h>
#include <process_image.h>
#include <motor_control.h>
#include <blinker.h>
#include <audio/play_melody.h>
#include <audio/audio_thread.h>
#include <prox_sensors.h>

static bool partyMusic = false;

static THD_WORKING_AREA(Music_wa, 8192);
static THD_FUNCTION(Music, arg) {
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
			call_blinker(STRAIGHT, barcode_num);
			switch (barcode_num) {
			case 1:
				playMelody(IMPOSSIBLE_MISSION, ML_FORCE_CHANGE, NULL);
				break;
			case 2:
				playMelody(WE_ARE_THE_CHAMPIONS, ML_FORCE_CHANGE, NULL);
				break;
			case 3:
				playMelody(MARIO, ML_FORCE_CHANGE, NULL);
				break;
			case 4:
				playMelody(PIRATES_OF_THE_CARIBBEAN, ML_FORCE_CHANGE, NULL);
				break;
			case 5:
				playMelody(STARWARS, ML_FORCE_CHANGE, NULL);
				break;
			case 6:
				playMelody(SANDSTORMS, ML_FORCE_CHANGE, NULL);
				break;
			default:
				break;
			}
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
	chThdCreateStatic(Music_wa, sizeof(Music_wa),
	NORMALPRIO, Music, NULL);
}

void party_music(void) {
	partyMusic = true;
}
