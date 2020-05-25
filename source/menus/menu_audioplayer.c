#include <psp2/audioout.h>
#include <psp2/appmgr.h>
#include <psp2/io/dirent.h>
#include <psp2/kernel/clib.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/power.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <psp2/shellutil.h>
#include <shellaudio.h>

#include "audio.h"
#include "common.h"
#include "config.h"
#include "fs.h"
#include "menu_displayfiles.h"
#include "touch.h"
#include "textures.h"
#include "utils.h"
#include "dirbrowse.h"

#define METADATA1_X 15
#define METADATA2_X 495
#define METADATA_Y 155

typedef enum {
	MUSIC_STATE_NONE,   // 0
	MUSIC_STATE_REPEAT, // 1
	MUSIC_STATE_REPEAT_ALL, // 2
	MUSIC_STATE_SHUFFLE // 3
} Music_State;

static char playlist[1024][512];
static int count = 0, selection = 0, initial_selection = 0, state = 0;
static int length_time_width = 0;
char *position_time = NULL, *length_time = NULL, *filename = NULL;
static SceBool isFinishedPlaylist = SCE_FALSE;
static SceBool isFirstTimeInit = SCE_TRUE;

extern int isFG;
extern void* mspace;
extern SceBool isSceShellUsed;

void* sceClibMspaceCalloc(void* space, size_t num, size_t size);
int sceAudioOutSetEffectType(int type);

static int Menu_GetMusicList(void) {
	SceUID dir = 0;

	if (R_SUCCEEDED(dir = sceIoDopen(cwd))) {
		int entryCount = 0, i = 0;
		SceIoDirent *entries = (SceIoDirent *)sceClibMspaceCalloc(mspace, MAX_FILES, sizeof(SceIoDirent));

		while (sceIoDread(dir, &entries[entryCount]) > 0)
			entryCount++;

		sceIoDclose(dir);
		qsort(entries, entryCount, sizeof(SceIoDirent), Utils_Alphasort);

		for (i = 0; i < entryCount; i++) {
			if ((!sceClibStrncasecmp(FS_GetFileExt(entries[i].d_name), "flac", 4)) || (!sceClibStrncasecmp(FS_GetFileExt(entries[i].d_name), "it", 4)) ||
				(!sceClibStrncasecmp(FS_GetFileExt(entries[i].d_name), "mod", 4)) || (!sceClibStrncasecmp(FS_GetFileExt(entries[i].d_name), "mp3", 4)) || 
				(!sceClibStrncasecmp(FS_GetFileExt(entries[i].d_name), "ogg", 4)) || (!sceClibStrncasecmp(FS_GetFileExt(entries[i].d_name), "opus", 4)) ||
				(!sceClibStrncasecmp(FS_GetFileExt(entries[i].d_name), "s3m", 4)) || (!sceClibStrncasecmp(FS_GetFileExt(entries[i].d_name), "wav", 4)) || 
				(!sceClibStrncasecmp(FS_GetFileExt(entries[i].d_name), "xm", 4)) || (!sceClibStrncasecmp(FS_GetFileExt(entries[i].d_name), "at9", 4)) ||
				(!sceClibStrncasecmp(FS_GetFileExt(entries[i].d_name), "m4a", 4)) || (!sceClibStrncasecmp(FS_GetFileExt(entries[i].d_name), "aac", 4))) {
				strcpy(playlist[count], cwd);
				strcpy(playlist[count] + strlen(playlist[count]), entries[i].d_name);
				count++;
			}
		}

		sceClibMspaceFree(mspace, entries);
	}
	else {
		sceIoDclose(dir);
		return dir;
	}

	return 0;
}

static int Music_GetCurrentIndex(char *path) {
	for(int i = 0; i < count; ++i) {
		if (!sceClibStrcmp(playlist[i], path))
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
		sceClibSnprintf(string, 35, "%02d:%02d:%02d", h, m, s);
	else
		sceClibSnprintf(string, 35, "%02d:%02d", m, s);
}

static void Menu_InitMusic(char *path) {
	Audio_Init(path);

	if (!isSceShellUsed)
		sceAudioOutSetAlcMode(config.alc_mode);
	else
		shellAudioSetALCModeForMusicPlayer(config.alc_mode);

	if (!isSceShellUsed)
		sceAudioOutSetEffectType(config.eq_mode);
	else
		shellAudioSetEQModeForMusicPlayer(config.eq_mode);

	filename = sceClibMspaceMalloc(mspace, 128);
	sceClibSnprintf(filename, 128, Utils_Basename(path));
	position_time = sceClibMspaceMalloc(mspace, 35);
	length_time = sceClibMspaceMalloc(mspace, 35);
	length_time_width = 0;

	Menu_ConvertSecondsToString(length_time, Audio_GetLengthSeconds());
	length_time_width = vita2d_font_text_width(font, 25, length_time);
	selection = Music_GetCurrentIndex(path);

	if (isFirstTimeInit) {
		initial_selection = selection;
		isFirstTimeInit = SCE_FALSE;
	}
}

static void Music_HandleNext(SceBool forward, int state) {

	SceBool repeat_check = SCE_FALSE;

	if (state == MUSIC_STATE_NONE) {
		if (forward)
			selection++;
		else
			selection--;
		repeat_check = SCE_TRUE;
	}
	else if (state == MUSIC_STATE_REPEAT_ALL) {
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

	if (selection == initial_selection && repeat_check)
		isFinishedPlaylist = SCE_TRUE;

	Audio_Stop();

	sceClibMspaceFree(mspace, filename);
	sceClibMspaceFree(mspace, length_time);
	sceClibMspaceFree(mspace, position_time);

	Audio_Term();
	Menu_InitMusic(playlist[selection]);
}

void Menu_PlayAudio(char *path) {

	Menu_GetMusicList();
	Menu_InitMusic(path);

	Utils_LockPower();
	vita2d_set_clear_color(RGBA8(53, 63, 73, 255));

	int seek_detect = -1, selection = 0, max_items = 4;
	SceBool isInSettings = SCE_FALSE;

	const char *menu_items[] = {
		"Off",
		"Heavy",
		"Pop",
		"Jazz",
		"Unique"
	};

	while(SCE_TRUE) {
		if (isFG && !isInSettings) {
			vita2d_start_drawing();
			vita2d_clear_screen();

			vita2d_draw_texture(icon_back, BTN_BACK_X, BTN_TOPBAR_Y + 4);

			vita2d_draw_fill_circle(BTN_SETTINGS_X, BTN_TOPBAR_Y + 9, 3, RGBA8(255, 255, 255, 255));
			vita2d_draw_fill_circle(BTN_SETTINGS_X, BTN_TOPBAR_Y + 24, 3, RGBA8(255, 255, 255, 255));
			vita2d_draw_fill_circle(BTN_SETTINGS_X, BTN_TOPBAR_Y + 39, 3, RGBA8(255, 255, 255, 255));

			vita2d_draw_rectangle(0, 124, 960, 400, RGBA8(45, 48, 50, 255)); // Draw info box

			if ((metadata.has_meta) && (metadata.title[0] != '\0') && (metadata.artist[0] != '\0')) {
				vita2d_font_draw_text(font, METADATA1_X, METADATA_Y, RGBA8(255, 255, 255, 255), 25, strupr(metadata.title));
				vita2d_font_draw_text(font, METADATA1_X, METADATA_Y + 30, RGBA8(255, 255, 255, 255), 25, strupr(metadata.artist));
			}
			else if ((metadata.has_meta) && (metadata.title[0] != '\0'))
				vita2d_font_draw_text(font, METADATA1_X, METADATA_Y + (80 - vita2d_font_text_height(font, 25, strupr(metadata.title))) + 15, RGBA8(255, 255, 255, 255), 25, strupr(metadata.title));
			else
				vita2d_font_draw_text(font, METADATA1_X, METADATA_Y + (80 - vita2d_font_text_height(font, 25, strupr(filename))) + 15, RGBA8(255, 255, 255, 255), 25, filename);

			if ((metadata.has_meta) && (metadata.album[0] != '\0'))
				vita2d_font_draw_textf(font, METADATA2_X, METADATA_Y, RGBA8(255, 255, 255, 255), 25, "Album: %s\n", metadata.album);

			if ((metadata.has_meta) && (metadata.year[0] != '\0'))
				vita2d_font_draw_textf(font, METADATA2_X, METADATA_Y + 30, RGBA8(255, 255, 255, 255), 25, "Year: %s\n", metadata.year);

			if ((metadata.has_meta) && (metadata.genre[0] != '\0'))
				vita2d_font_draw_textf(font, METADATA2_X, METADATA_Y + 60, RGBA8(255, 255, 255, 255), 25, "Genre: %s\n", metadata.genre);

			if (!Audio_IsPaused())
				vita2d_draw_texture(btn_pause, BTN_PLAY_X, BTN_MAIN_Y); // Playing
			else
				vita2d_draw_texture(btn_play, BTN_PLAY_X, BTN_MAIN_Y); // Paused

			vita2d_draw_texture(btn_rewind, BTN_REW_X, BTN_MAIN_Y);
			vita2d_draw_texture(btn_forward, BTN_FF_X, BTN_MAIN_Y);

			switch (state) {
			case MUSIC_STATE_SHUFFLE:
				vita2d_draw_texture(btn_shuffle, BTN_SHUFFLE_X, BTN_SUB_Y);
				break;
			default:
				vita2d_draw_texture_tint(btn_shuffle, BTN_SHUFFLE_X, BTN_SUB_Y, RGBA8(127, 138, 132, 255));
				break;
			}
			switch (state) {
			case MUSIC_STATE_REPEAT:
				vita2d_draw_texture(btn_repeat, BTN_REPEAT_X, BTN_SUB_Y);
				vita2d_draw_fill_circle(BTN_REPEAT_X + BUTTON_WIDTH / 2, BTN_SUB_Y + BUTTON_HEIGHT / 2, 5, RGBA8(255, 255, 255, 255));
				break;
			case MUSIC_STATE_REPEAT_ALL:
				vita2d_draw_texture(btn_repeat, BTN_REPEAT_X, BTN_SUB_Y);
				break;
			default:
				vita2d_draw_texture_tint(btn_repeat, BTN_REPEAT_X, BTN_SUB_Y, RGBA8(127, 138, 132, 255));
				break;
			}

			Menu_ConvertSecondsToString(position_time, Audio_GetPositionSeconds());
			vita2d_font_draw_text(font, SEEK_X, 480, RGBA8(255, 255, 255, 255), 25, position_time);
			vita2d_font_draw_text(font, 910 - length_time_width, 480, RGBA8(255, 255, 255, 255), 25, length_time);
			vita2d_draw_rectangle(SEEK_X, 490, SEEK_WIDTH, 4, RGBA8(97, 97, 97, 255));
			vita2d_draw_rectangle(SEEK_X, 490, (((double)Audio_GetPosition() / (double)Audio_GetLength()) * SEEK_WIDTH_FLOAT), 4, RGBA8(255, 255, 255, 255));

			vita2d_end_drawing();
			vita2d_wait_rendering_done();
			vita2d_end_shfb();

			Utils_ReadControls();
			Touch_Update();

			if (pressed & SCE_CTRL_ENTER)
				Audio_Pause();

			seek_detect = Touch_GetDragRecStateXPos(TOUCHREC_DRAG_SEEK);

			if (seek_detect > -1) {
				Touch_ChangeRecRectangle(TOUCHREC_DRAG_SEEK, SEEK_X * 2, SEEK_WIDTH * 2, 0, 1088);
				Audio_SetSeekMode(SCE_TRUE);
				Audio_SetSeekPosition(seek_detect - SEEK_X);
			}

			if (seek_detect == -1 && Audio_GetSeekMode()) {
				// Pause first if not paused.
				if (!Audio_IsPaused() && !isSceShellUsed)
					Audio_Pause();

				Audio_Seek();

				// Unpause.
				if (Audio_IsPaused() && !isSceShellUsed)
					Audio_Pause();

				Touch_ChangeRecRectangle(TOUCHREC_DRAG_SEEK, SEEK_X * 2, SEEK_WIDTH * 2, 960, 50);
				Audio_SetSeekPosition(0);
				Audio_SetSeekMode(SCE_FALSE);
			}

			if ((pressed & SCE_CTRL_TRIANGLE) || Touch_GetTapRecState(TOUCHREC_TAP_SHUFFLE)) {
				if (state == MUSIC_STATE_SHUFFLE)
					state = MUSIC_STATE_NONE;
				else
					state = MUSIC_STATE_SHUFFLE;
			}
			else if ((pressed & SCE_CTRL_SQUARE) || Touch_GetTapRecState(TOUCHREC_TAP_REPEAT)) {
				if (state == MUSIC_STATE_REPEAT)
					state = MUSIC_STATE_NONE;
				else if (state == MUSIC_STATE_REPEAT_ALL)
					state = MUSIC_STATE_REPEAT;
				else
					state = MUSIC_STATE_REPEAT_ALL;
			}

			if ((pressed & SCE_CTRL_LTRIGGER) || Touch_GetTapRecState(TOUCHREC_TAP_REW)) {
				if (count != 0)
					Music_HandleNext(SCE_FALSE, MUSIC_STATE_REPEAT_ALL);
			}
			else if ((pressed & SCE_CTRL_RTRIGGER) || Touch_GetTapRecState(TOUCHREC_TAP_FF)) {
				if (count != 0)
					Music_HandleNext(SCE_TRUE, MUSIC_STATE_REPEAT_ALL);
			}

			if ((pressed & SCE_CTRL_CANCEL) || Touch_GetTapRecState(TOUCHREC_TAP_BACK))
				break;

			if (Touch_GetTapRecState(TOUCHREC_TAP_PLAY))
				Audio_Pause();

			if ((pressed & SCE_CTRL_SELECT) || Touch_GetTapRecState(TOUCHREC_TAP_SETTINGS)) {
				vita2d_set_clear_color(RGBA8(250, 250, 250, 255));
				isInSettings = SCE_TRUE;
			}
		}
		else if (isFG && isInSettings) {
			vita2d_start_drawing();
			vita2d_clear_screen();

			vita2d_draw_rectangle(0, 0, 960, 112, RGBA8(51, 51, 51, 255));

			vita2d_draw_texture(icon_back, BTN_BACK_X, BTN_TOPBAR_Y);
			vita2d_font_draw_text(font, 102, 40 + ((72 - vita2d_font_text_height(font, 25, "Equalizer")) / 2) + 20, RGBA8(255, 255, 255, 255), 25, "Equalizer");

			int printed = 0;

			for (int i = 0; i < max_items + 1; i++) {
				if (printed == FILES_PER_PAGE)
					break;

				if (selection < FILES_PER_PAGE || i >(selection - FILES_PER_PAGE)) {
					if (i == selection)
						vita2d_draw_rectangle(0, 112 + (72 * printed), 960, 72, RGBA8(230, 230, 230, 255));

					vita2d_font_draw_text(font, 30, 120 + (72 / 2) + (72 * printed), RGBA8(51, 51, 51, 255), 25, menu_items[i]);

					printed++;
				}
			}

			vita2d_draw_texture(config.eq_mode == 0 ? radio_on : radio_off, 850, 126);
			vita2d_draw_texture(config.eq_mode == 1 ? radio_on : radio_off, 850, 198);
			vita2d_draw_texture(config.eq_mode == 2 ? radio_on : radio_off, 850, 270);
			vita2d_draw_texture(config.eq_mode == 3 ? radio_on : radio_off, 850, 342);
			vita2d_draw_texture(config.eq_mode == 4 ? radio_on : radio_off, 850, 414);

			vita2d_end_drawing();
			vita2d_wait_rendering_done();
			vita2d_end_shfb();

			Utils_ReadControls();
			Touch_Update();

			if ((pressed & SCE_CTRL_CANCEL) || Touch_GetTapRecState(TOUCHREC_TAP_BACK)) {
				vita2d_set_clear_color(RGBA8(53, 63, 73, 255));
				isInSettings = SCE_FALSE;
			}

			if (pressed & SCE_CTRL_UP)
				selection--;
			else if (pressed & SCE_CTRL_DOWN)
				selection++;

			Utils_SetMax(&selection, 0, max_items);
			Utils_SetMin(&selection, max_items, 0);

			if (pressed & SCE_CTRL_ENTER) {
				config.eq_mode = selection;
				Config_Save(config);

				if (!isSceShellUsed)
					sceAudioOutSetEffectType(config.eq_mode);
				else
					shellAudioSetEQModeForMusicPlayer(config.eq_mode);

				Dirbrowse_PopulateFiles(SCE_TRUE);
			}
		}
		else {
			if (isSceShellUsed)
				Audio_GetPosition();
			sceKernelDelayThread(10 * 1000);
		}

		if (!playing) {
			if (state == MUSIC_STATE_NONE) {
				if (count != 0)
					Music_HandleNext(SCE_TRUE, MUSIC_STATE_NONE);
			}
			else if (state == MUSIC_STATE_REPEAT)
				Music_HandleNext(SCE_FALSE, MUSIC_STATE_REPEAT);
			else if (state == MUSIC_STATE_REPEAT_ALL) {
				if (count != 0)
					Music_HandleNext(SCE_TRUE, MUSIC_STATE_REPEAT_ALL);
			}
			else if (state == MUSIC_STATE_SHUFFLE) {
				if (count != 0)
					Music_HandleNext(SCE_FALSE, MUSIC_STATE_SHUFFLE);
			}
		}

		if (Utils_AppStatusIsRunning())
			break;

		if (isFinishedPlaylist)
			break;
	}

	sceClibMspaceFree(mspace, filename);
	sceClibMspaceFree(mspace, length_time);
	sceClibMspaceFree(mspace, position_time);

	Audio_Stop();
	Audio_Term();
	isFirstTimeInit = SCE_TRUE;
	isFinishedPlaylist = SCE_FALSE;
	count = 0;
	initial_selection = 0;
	Utils_UnlockPower();
	Menu_DisplayFiles();
}
