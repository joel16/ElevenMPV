#include <psp2/appmgr.h>
#include <psp2/io/dirent.h>
#include <psp2/power.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "audio.h"
#include "common.h"
#include "fs.h"
#include "menu_displayfiles.h"
#include "status_bar.h"
#include "touch.h"
#include "textures.h"
#include "utils.h"

#define BUTTON_WIDTH  68
#define BUTTON_HEIGHT 68

typedef enum {
	MUSIC_STATE_NONE,   // 0
	MUSIC_STATE_REPEAT, // 1
	MUSIC_STATE_SHUFFLE // 2
} Music_State;

static char playlist[1024][512];
static int count = 0, selection = 0, state = 0;
static int length_time_width = 0;
char *position_time = NULL, *length_time = NULL, *filename = NULL;

static int Menu_GetMusicList(void) {
	SceUID dir = 0;

	if (R_SUCCEEDED(dir = sceIoDopen(cwd))) {
		int entryCount = 0, i = 0;
		SceIoDirent *entries = (SceIoDirent *)calloc(MAX_FILES, sizeof(SceIoDirent));

		while (sceIoDread(dir, &entries[entryCount]) > 0)
			entryCount++;

		sceIoDclose(dir);
		qsort(entries, entryCount, sizeof(SceIoDirent), Utils_Alphasort);

		for (i = 0; i < entryCount; i++) {
			if ((!strncasecmp(FS_GetFileExt(entries[i].d_name), "flac", 4)) || (!strncasecmp(FS_GetFileExt(entries[i].d_name), "it", 2)) || 
				(!strncasecmp(FS_GetFileExt(entries[i].d_name), "mod", 3)) || (!strncasecmp(FS_GetFileExt(entries[i].d_name), "mp3", 3)) || 
				(!strncasecmp(FS_GetFileExt(entries[i].d_name), "ogg", 3)) || (!strncasecmp(FS_GetFileExt(entries[i].d_name), "opus", 4)) ||
				(!strncasecmp(FS_GetFileExt(entries[i].d_name), "s3m", 3)) || (!strncasecmp(FS_GetFileExt(entries[i].d_name), "wav", 3)) || 
				(!strncasecmp(FS_GetFileExt(entries[i].d_name), "xm", 2))) {
				strcpy(playlist[count], cwd);
				strcpy(playlist[count] + strlen(playlist[count]), entries[i].d_name);
				count++;
			}
		}

		free(entries);
	}
	else {
		sceIoDclose(dir);
		return dir;
	}

	return 0;
}

static int Music_GetCurrentIndex(char *path) {
	for(int i = 0; i < count; ++i) {
		if (!strcmp(playlist[i], path))
			return i;
	}

	return 0;
}

static void Menu_ConvertSecondsToString(char *string, SceUInt64 seconds) {
	int h = 0, m = 0, s = 0;
	h = (seconds / 3600);
	m = (seconds - (3600 * h)) / 60;
	s = (seconds - (3600 * h) - (m * 60));

	if (h > 0)
		snprintf(string, 35, "%02d:%02d:%02d", h, m, s);
	else
		snprintf(string, 35, "%02d:%02d", m, s);
}

static void Menu_InitMusic(char *path) {
	Audio_Init(path);

	filename = malloc(128);
	snprintf(filename, 128, Utils_Basename(path));
	position_time = malloc(35);
	length_time = malloc(35);
	length_time_width = 0;

	Menu_ConvertSecondsToString(length_time, Audio_GetLengthSeconds());
	length_time_width = vita2d_font_text_width(font, 25, length_time);
	selection = Music_GetCurrentIndex(path);
}

static void Music_HandleNext(SceBool forward, int state) {
	if (state == MUSIC_STATE_NONE) {
		if (forward)
			selection++;
		else
			selection--;
	}
	else if (state == MUSIC_STATE_SHUFFLE) {
		int old_selection = selection;
		time_t t;
		srand((unsigned) time(&t));
		selection = rand() % (count - 1);

		if (selection == old_selection)
			selection--;
	}

	Utils_SetMax(&selection, 0, (count - 1));
	Utils_SetMin(&selection, (count - 1), 0);

	Audio_Stop();

	free(filename);
	free(length_time);
	free(position_time);

	if ((metadata.has_meta) && (metadata.cover_image)) {
		vita2d_wait_rendering_done();
		vita2d_free_texture(metadata.cover_image);
	}

	Audio_Term();
	Menu_InitMusic(playlist[selection]);
}

void Menu_PlayAudio(char *path) {
	Menu_GetMusicList();
	Menu_InitMusic(path);

	sceAppMgrAcquireBgmPort();
	Utils_LockPower();

	while(SCE_TRUE) {
		vita2d_start_drawing();
		vita2d_clear_screen();

		vita2d_draw_texture(default_artwork_blur, 0, 0);
		vita2d_draw_rectangle(0, 0, 960, 40, RGBA8(97, 97, 97, 255));
		vita2d_draw_texture(icon_back, 10, 60);
		StatusBar_Display();

		if ((metadata.has_meta) && (metadata.title[0] != '\0') && (metadata.artist[0] != '\0')) {
			vita2d_font_draw_text(font, 80, 22 + (80 - vita2d_font_text_height(font, 25, strupr(metadata.title))) + 20, RGBA8(255, 255, 255, 255), 25, strupr(metadata.title));
			vita2d_font_draw_text(font, 80, 22 + (80 - vita2d_font_text_height(font, 25, strupr(metadata.artist))) + 55, RGBA8(255, 255, 255, 255), 25, strupr(metadata.artist));
		}
		else if ((metadata.has_meta) && (metadata.title[0] != '\0'))
			vita2d_font_draw_text(font, 80, 22 + (80 - vita2d_font_text_height(font, 25, strupr(metadata.title))) + 15, RGBA8(255, 255, 255, 255), 25, strupr(metadata.title));
		else
			vita2d_font_draw_text(font, 80, 22 + (80 - vita2d_font_text_height(font, 25, strupr(filename))) + 15, RGBA8(255, 255, 255, 255), 25, filename);

		vita2d_draw_rectangle(0, 124, 400, 400, RGBA8(97, 97, 97, 255));

		if ((metadata.has_meta) && (metadata.cover_image))
			vita2d_draw_texture_scale(metadata.cover_image, 0, 124, 400.0f/vita2d_texture_get_width(metadata.cover_image), 400.0f/vita2d_texture_get_height(metadata.cover_image));
		else
			vita2d_draw_texture(default_artwork, 0, 124); // Default album art

		vita2d_draw_rectangle(410, 124, 550, 400, RGBA8(45, 48, 50, 255)); // Draw info box (outer)
		vita2d_draw_rectangle(420, 134, 530, 380, RGBA8(46, 49, 51, 255)); // Draw info box (inner)

		if (!Audio_IsPaused())
			vita2d_draw_texture(btn_pause, 410 + ((550 - BUTTON_WIDTH) / 2), 124 + ((400 - BUTTON_HEIGHT) / 2)); // Playing
		else
			vita2d_draw_texture(btn_play, 410 + ((550 - BUTTON_WIDTH) / 2), 124 + ((400 - BUTTON_HEIGHT) / 2)); // Paused

		vita2d_draw_texture(btn_rewind, 410 + ((550 - BUTTON_WIDTH) / 2) - 136, 124 + ((400 - BUTTON_HEIGHT) / 2));
		vita2d_draw_texture(btn_forward, 410 + ((550 - BUTTON_WIDTH) / 2) + 136, 124 + ((400 - BUTTON_HEIGHT) / 2));

		vita2d_draw_texture(state == MUSIC_STATE_SHUFFLE? btn_shuffle_overlay : btn_shuffle, 410 + ((550 - BUTTON_WIDTH) / 2) - 90, 124 + ((400 - BUTTON_HEIGHT) / 2) + 100);
		vita2d_draw_texture(state == MUSIC_STATE_REPEAT? btn_repeat_overlay : btn_repeat, 410 + ((550 - BUTTON_WIDTH) / 2) + 90, 124 + ((400 - BUTTON_HEIGHT) / 2) + 100);

		Menu_ConvertSecondsToString(position_time, Audio_GetPositionSeconds());
		vita2d_font_draw_text(font, 460, 480, RGBA8(255, 255, 255, 255), 25, position_time);
		vita2d_font_draw_text(font, 910 - length_time_width, 480, RGBA8(255, 255, 255, 255), 25, length_time);
		vita2d_draw_rectangle(460, 490, 450, 4, RGBA8(97, 97, 97, 255));
		vita2d_draw_rectangle(460, 490, (((double)Audio_GetPosition()/(double)Audio_GetLength()) * 450.0), 4, RGBA8(255, 255, 255, 255));

		vita2d_end_drawing();
		vita2d_swap_buffers();

		if (!playing) {
			if (state == MUSIC_STATE_NONE) {
				if (count != 0)
					Music_HandleNext(SCE_TRUE, MUSIC_STATE_NONE);
			}
			else if (state == MUSIC_STATE_REPEAT)
				Music_HandleNext(SCE_FALSE, MUSIC_STATE_REPEAT);
			else if (state == MUSIC_STATE_SHUFFLE) {
				if (count != 0)
					Music_HandleNext(SCE_FALSE, MUSIC_STATE_SHUFFLE);
			}
		}

		Utils_ReadControls();
		Touch_Update();

		if (pressed & SCE_CTRL_ENTER)
			Audio_Pause();

		if (pressed & SCE_CTRL_TRIANGLE) {
			if (state == MUSIC_STATE_SHUFFLE)
				state = MUSIC_STATE_NONE;
			else
				state = MUSIC_STATE_SHUFFLE;
		}
		else if (pressed & SCE_CTRL_SQUARE) {
			if (state == MUSIC_STATE_REPEAT)
				state = MUSIC_STATE_NONE;
			else
				state = MUSIC_STATE_REPEAT;
		}

		if (pressed & SCE_CTRL_LTRIGGER) {
			if (count != 0)
				Music_HandleNext(SCE_FALSE, MUSIC_STATE_NONE);
		}
		else if (pressed & SCE_CTRL_RTRIGGER) {
			if (count != 0)
				Music_HandleNext(SCE_TRUE, MUSIC_STATE_NONE);
		}

		if (pressed & SCE_CTRL_START)
			scePowerRequestDisplayOff();

		if (pressed & SCE_CTRL_CANCEL)
			break;

		if (Touch_Position((410 + ((550 - BUTTON_WIDTH) / 2)), (124 + ((400 - BUTTON_HEIGHT) / 2)), ((410 + ((550 - BUTTON_WIDTH) / 2)) + BUTTON_WIDTH), 
			((124 + ((400 - BUTTON_HEIGHT) / 2)) + BUTTON_HEIGHT)))
			Audio_Pause();

		if (Touch_Position(10, 57, 60, 107))
			break;

		if (Touch_Position((410 + ((550 - BUTTON_WIDTH) / 2) - 136), (124 + ((400 - BUTTON_HEIGHT) / 2)), ((410 + ((550 - BUTTON_WIDTH) / 2) - 136) + BUTTON_WIDTH), 
			((124 + ((400 - BUTTON_HEIGHT) / 2)) + BUTTON_HEIGHT))) {
			if (count != 0)
				Music_HandleNext(SCE_FALSE, MUSIC_STATE_NONE);
		}
		else if (Touch_Position((410 + ((550 - BUTTON_WIDTH) / 2) + 136), (124 + ((400 - BUTTON_HEIGHT) / 2)), ((410 + ((550 - BUTTON_WIDTH) / 2) + 136) + BUTTON_WIDTH), 
			((124 + ((400 - BUTTON_HEIGHT) / 2)) + BUTTON_HEIGHT))) {
			if (count != 0)
				Music_HandleNext(SCE_TRUE, MUSIC_STATE_NONE);
		}

		if (Touch_Position((410 + ((550 - BUTTON_WIDTH) / 2) - 90), (124 + ((400 - BUTTON_HEIGHT) / 2) + 100), ((410 + ((550 - BUTTON_WIDTH) / 2) - 90) + BUTTON_WIDTH), 
			((124 + ((400 - BUTTON_HEIGHT) / 2) + 100) + BUTTON_HEIGHT))) {
			if (state == MUSIC_STATE_SHUFFLE)
				state = MUSIC_STATE_NONE;
			else
				state = MUSIC_STATE_SHUFFLE;
		}
		else if (Touch_Position((410 + ((550 - BUTTON_WIDTH) / 2) + 90), (124 + ((400 - BUTTON_HEIGHT) / 2) + 100), ((410 + ((550 - BUTTON_WIDTH) / 2) + 90) + BUTTON_WIDTH), 
			((124 + ((400 - BUTTON_HEIGHT) / 2) + 100) + BUTTON_HEIGHT))) {
			if (state == MUSIC_STATE_REPEAT)
				state = MUSIC_STATE_NONE;
			else
				state = MUSIC_STATE_REPEAT;
		}
	}

	free(filename);
	free(length_time);
	free(position_time);

	if ((metadata.has_meta) && (metadata.cover_image)) {
		vita2d_wait_rendering_done();
		vita2d_free_texture(metadata.cover_image);
	}

	Audio_Stop();
	Audio_Term();
	count = 0;
	Touch_Reset();
	Utils_UnlockPower();
	Menu_DisplayFiles();
}
