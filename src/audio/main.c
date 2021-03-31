#include <ampi.h>
#include <ampienv.h>

#include <linux/coroutine.h>
#include <linux/synchronize.h>
#include "common.h"
#include "gpio.h"
#include "gpioextra.h"
#include "printf.h"
#include "audio_sequence.h"
#include "timer.h"
#include "read_angle.h"
#include "gl.h"
#include "config.h"

// Implemented in synth.c
unsigned synth(int16_t **buf, unsigned chunk_size);

// INIT_AUDIO(hello_world);
INIT_AUDIO(hihat);
INIT_AUDIO(snare);
INIT_AUDIO(crash);
INIT_AUDIO(kick);

static unsigned int snprintf_angle(double angle, char* buf, size_t buflen, unsigned int precision) {
	char temp[16];
	size_t i = 0;
	if (angle < 0) {
		temp[i++] = '-';
		angle *= -1;
	}
	i += snprintf(&temp[i], sizeof(temp) - i, "%d", (int) angle);
	angle -= (int) angle;
	if (i < sizeof(temp) - 1) temp[i++] = '.';
	int pow = 1;
	while (precision--) pow *= 10;
	i += snprintf(&temp[i], i < sizeof(temp) ? sizeof(temp) - i : 0, "%d", (int) (angle * pow + 0.5));
	snprintf(buf, buflen, "%s", temp);
	return i;
}

void main (void)
{
	// Start the timer in the environment
	env_init();

	// Initialize audio
	DSB();
	// linuxemu_EnterCritical();  // I don't know if this is necessary
	AMPiInitialize(SAMPLE_RATE, 800);
	AMPiSetChunkCallback(synth);
	// linuxemu_LeaveCritical();
	DMB();


	printf("Finished initializing audio\n");
	DSB();
#ifndef DEBUG_NO_AUDIO
	AMPiStart();
#endif
	timer_delay(1);
	printf("AMPi ready!\n");


    // lsm6ds33_init(LSM6DS33_I2CADDR_DEFAULT, LSM6DS33_RATE_104_HZ);
    lsm6ds33_init_dual(LSM6DS33_I2CADDR_DEFAULT, LSM6DS33_I2CADDR_ALTERNATE, LSM6DS33_RATE_208_HZ);
	printf("Finished initializing sensor\n");
    gesture_handler_t reader0 = createGestureReader(CONFIG_HORIZ, CONFIG_VERT);
	gesture_handler_t reader1 = createGestureReader(CONFIG_HORIZ, CONFIG_VERT);
    lsm6ds33_set_active_sensor(LSM6DS33_SENSOR0);

#ifdef SHOULD_SENSOR_CALIBRATE
	calibrate(&reader0);
	lsm6ds33_set_active_sensor(LSM6DS33_SENSOR1);
	calibrate(&reader1);
    lsm6ds33_set_active_sensor(LSM6DS33_SENSOR0);	
	printf("Done calibrating\n");
#endif


	// setup buttons
	gpio_set_input(BUTTON0_PIN);
	gpio_set_input(BUTTON1_PIN);
	gpio_set_pullup(BUTTON0_PIN);
	gpio_set_pullup(BUTTON1_PIN);

	struct audio_sequence hello_world_hihat;  // no hello world, too large, just followed by hihat
	hello_world_hihat.index = 0;
	hello_world_hihat.len = 0;
	hello_world_hihat.audios[hello_world_hihat.len++] = MAKE_AUDIO(hihat, 5.0);
	addTrack(&hello_world_hihat);

	struct audio_sequence snare_only;
	snare_only.index = snare_only.len = 0;
	snare_only.audios[snare_only.len++] = MAKE_AUDIO(snare, 1.0);

	// struct audio_sequence hihat_snare;
	// hihat_snare.index = hihat_snare.len = 0;
	// hihat_snare.audios[hihat_snare.len++] = MAKE_AUDIO(hihat, 3.0);
	// hihat_snare.audios[hihat_snare.len++] = MAKE_SILENCE(0.5);
	// hihat_snare.audios[hihat_snare.len++] = MAKE_AUDIO(hihat, 5.0);
	// hihat_snare.audios[hihat_snare.len++] = MAKE_AUDIO(snare, 1);

	struct audio_sequence kick_drum;
	kick_drum.index = kick_drum.len = 0;
	kick_drum.audios[kick_drum.len++] = MAKE_AUDIO(kick, 1.0);

	struct audio_sequence crash_cymbal;
	crash_cymbal.index = crash_cymbal.len = 0;
	crash_cymbal.audios[crash_cymbal.len++] = MAKE_AUDIO(crash, 3.0);

#ifdef DEBUG_NO_AUDIO
	printf("Initializing graphics\n");
	linuxemu_EnterCritical();
	const size_t WIDTH = 1132, HEIGHT = 640;
	gl_init(1132, 640, GL_SINGLEBUFFER);
	// Draw 0 line
	gl_clear(GL_BLACK);
	gl_draw_rect(0, HEIGHT / 2 - 2, WIDTH, 4, GL_WHITE);
	linuxemu_LeaveCritical();
	int pixelIndex = 0;
#endif
	unsigned int printTime = timer_get_ticks();

	printf("Done\n\n");  // So that the angle doesn't overwrite anything
	while (1) {
		lsm6ds33_data_t data;

        lsm6ds33_set_active_sensor(LSM6DS33_SENSOR0);
        lsm6ds33_get_all(&data);
		updateAngle(&reader0, &data);

        lsm6ds33_set_active_sensor(LSM6DS33_SENSOR1);
        lsm6ds33_get_all(&data);
		updateAngle(&reader1, &data);

		unsigned int time = timer_get_ticks();
		if (time - printTime > 10000) {
			char buf[32];
			int i = snprintf_angle(reader0.angle, buf, sizeof(buf), 2);
			while (i < sizeof(buf) - 1) buf[i++] = ' ';
			buf[i] = '\0';
			printf("\rAlpha: %s", buf);

#ifdef DEBUG_NO_AUDIO
			int y = -reader0.angle / 90 * HEIGHT + HEIGHT / 2;
			int yaccel = -(reader0.alpha / 50000) * HEIGHT + HEIGHT / 2;
			gl_draw_pixel(pixelIndex, yaccel, GL_RED);
			gl_draw_pixel(pixelIndex++, y, GL_BLUE);
			if (pixelIndex >= WIDTH) {
				pixelIndex = 0;
				gl_clear(GL_BLACK);
				gl_draw_rect(0, HEIGHT / 2 - 2, WIDTH, 4, GL_WHITE);
			}
#endif
			printTime = time;
		}
        
		if (checkUpDownGesture(&reader0)) {
			addTrack((gpio_read(BUTTON0_PIN) ? &snare_only : &kick_drum)); // snare drum if not pressed, kick if pressed
			// Random color hack
#ifdef DEBUG_NO_AUDIO
			gl_draw_rect(0, 0, 20, 20, ((time * 0xcf25801d) ^ time) | 0xff000000);
#endif
		}

		if (checkUpDownGesture(&reader1)) {
			addTrack((gpio_read(BUTTON1_PIN) ? &hello_world_hihat : &crash_cymbal)); // hihat if not pressed, crash cymbal if pressed
			// Random color hack
#ifdef DEBUG_NO_AUDIO
			gl_draw_rect(0, 20, 20, 20, ((time * 0xcf25801d) ^ time) | 0xff000000);
#endif
		}

	}
	DMB();
}
