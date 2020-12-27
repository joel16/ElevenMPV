#include <psp2/audioout.h>
#include <psp2/appmgr.h>
#include <psp2/kernel/iofilemgr.h>
#include <psp2/kernel/clib.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/processmgr.h> 
#include <psp2/kernel/rng.h>
#include <psp2/notificationutil.h>
#include <psp2/power.h>
#include <psp2/shellsvc.h>
#include <psp2/vshbridge.h>
#include <shellaudio.h>
#include <stdlib.h>
#include <string.h>

#include "audio.h"
#include "common.h"
#include "config.h"
#include "fs.h"
#include "menu_displayfiles.h"
#include "touch.h"
#include "motion.h"
#include "textures.h"
#include "utils.h"
#include "dirbrowse.h"
#include "vitaaudiolib.h"

#define METADATA1_X 80
#define METADATA2_X 425
#define METADATA1_Y BTN_SETTINGS_Y + 15
#define METADATA2_Y 155

#define NOCOVER_X 200
#define NOCOVER_Y 324

typedef enum {
	MUSIC_STATE_NONE,   // 0
	MUSIC_STATE_REPEAT, // 1
	MUSIC_STATE_REPEAT_ALL, // 2
	MUSIC_STATE_SHUFFLE // 3
} Music_State;

static Audio_Metadata empty_metadata = { 0 };
static char playlist[1024][512];
static char order[10];
static char external_cover[512];
static int count = 0, selection = 0, initial_selection = 0, state = 0;
static int length_time_width = 0;
char *position_time = NULL, *length_time = NULL, *filename = NULL;
static SceBool isFirstTimeInit = SCE_TRUE, ext_cover_is_jpeg = SCE_FALSE, ext_cover_loaded = SCE_FALSE, tex_filter_on = SCE_FALSE;
static SceUInt64 length;
static vita2d_texture* external_cover_tmp;

extern SceUID event_flag_uid;

#ifdef DEBUG
extern SceAppMgrBudgetInfo budget_info;
#endif

int sceAudioOutSetEffectType(int type);
int sceAppMgrAcquireBgmPortForMusicPlayer(void);

static int Menu_GetMusicList(void) {
	SceUID dir = 0;

	sceClibMemset(&external_cover, 0, 512);

	if (R_SUCCEEDED(dir = sceIoDopen(cwd))) {

		int entryCount = 0, i = 0;
		SceIoDirent *entries = (SceIoDirent *)calloc(MAX_FILES, sizeof(SceIoDirent));

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
				(!sceClibStrncasecmp(FS_GetFileExt(entries[i].d_name), "m4a", 4)) || (!sceClibStrncasecmp(FS_GetFileExt(entries[i].d_name), "aac", 4)) ||
				(!sceClibStrncasecmp(FS_GetFileExt(entries[i].d_name), "oma", 4)) || (!sceClibStrncasecmp(FS_GetFileExt(entries[i].d_name), "aa3", 4)) ||
				(!sceClibStrncasecmp(FS_GetFileExt(entries[i].d_name), "at3", 4)) || (!sceClibStrncasecmp(FS_GetFileExt(entries[i].d_name), "gta", 4))) {
				strcpy(playlist[count], cwd);
				strcpy(playlist[count] + strlen(playlist[count]), entries[i].d_name);
				count++;
			}

			if (!sceClibStrncasecmp(FS_GetFileExt(entries[i].d_name), "jpg", 4) || !sceClibStrncasecmp(FS_GetFileExt(entries[i].d_name), "jpeg", 4)) {
				if (!sceClibStrncasecmp(entries[i].d_name, "cover", 5) || !sceClibStrncasecmp(entries[i].d_name, "folder", 6)) {
					sceClibSnprintf(external_cover, 512, "%s%s", cwd, entries[i].d_name);
					ext_cover_is_jpeg = SCE_TRUE;
				}
			}
			else if (!sceClibStrncasecmp(FS_GetFileExt(entries[i].d_name), "png", 4)) {
				if (!sceClibStrncasecmp(entries[i].d_name, "cover", 5) || !sceClibStrncasecmp(entries[i].d_name, "folder", 6))
					sceClibSnprintf(external_cover, 512, "%s%s", cwd, entries[i].d_name);
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

void Menu_LoadExternalCover(void) {
	if (external_cover[0] != '\0' && metadata.cover_image == NULL && !ext_cover_loaded) {
		if (ext_cover_is_jpeg) {
			vita2d_JPEG_ARM_decoder_initialize();
			metadata.cover_image = vita2d_load_JPEG_ARM_file(external_cover, 0, 0, 0, 0);
			vita2d_JPEG_ARM_decoder_finish();
			external_cover_tmp = metadata.cover_image;
			ext_cover_loaded = SCE_TRUE;
			metadata.has_meta = SCE_TRUE;
		}
		else {
			metadata.cover_image = vita2d_load_PNG_file(external_cover, 0);
			external_cover_tmp = metadata.cover_image;
			ext_cover_loaded = SCE_TRUE;
			metadata.has_meta = SCE_TRUE;
		}
	}

	if (ext_cover_loaded && !metadata.has_meta) {
		metadata.cover_image = external_cover_tmp;
		metadata.has_meta = SCE_TRUE;
	}
	else if (ext_cover_loaded && metadata.has_meta && metadata.cover_image == NULL)
		metadata.cover_image = external_cover_tmp;
}

void Menu_UnloadExternalCover(void) {
	if (ext_cover_loaded) {
		vita2d_wait_rendering_done();
		metadata.cover_image = external_cover_tmp;
		vita2d_free_texture(metadata.cover_image);
		metadata.cover_image = NULL;
		ext_cover_loaded = SCE_FALSE;
	}
}

void Menu_NotificationBegin(void) {
	SceNotificationUtilProgressInitParam init_param;
	sceClibMemset(&init_param, 0, sizeof(SceNotificationUtilProgressInitParam));

	sceClibStrncpy((char *)init_param.notificationText, "\xB9\xF8", 2);
	if (metadata.has_meta) {
		if (metadata.title[0] != '\0')
			Utils_Utf8ToUtf16(&init_param.notificationText[1], metadata.title);
		else
			Utils_Utf8ToUtf16(&init_param.notificationText[1], filename);
		if (metadata.artist[0] != '\0') {
			Utils_Utf8ToUtf16(init_param.notificationSubText, metadata.artist);
		}
	}
	else
		Utils_Utf8ToUtf16(&init_param.notificationText[1], filename);

	if (config.notify_mode == 1)
		Utils_Utf8ToUtf16(init_param.cancelDialogText, "Stop playback?");
	else
		Utils_Utf8ToUtf16(init_param.cancelDialogText, "Stop playback and close ElevenMPV-A?");

	init_param.eventHandler = Utils_NotificationEventHandler;
	Utils_NotificationProgressBegin(&init_param);
}

void Menu_NotificationUpdate(void) {

	int strlen;
	SceUInt64 position = Audio_GetPositionSeconds();
	Menu_ConvertSecondsToString(position_time, position);

	SceNotificationUtilProgressUpdateParam update_param;
	sceClibMemset(&update_param, 0, sizeof(SceNotificationUtilProgressUpdateParam));

	if (paused) {
		sceClibStrncpy((char *)update_param.notificationText, "\x9A\xF8", 2);
		if ((metadata.has_meta) && (metadata.title[0] != '\0'))
			Utils_Utf8ToUtf16(&update_param.notificationText[1], metadata.title);
		else
			Utils_Utf8ToUtf16(&update_param.notificationText[1], filename);
	}

	char time[0x3F];
	sceClibMemset(&time, 0, 0x3F);

	if (metadata.has_meta) {
		if (metadata.artist[0] != '\0') {
			strlen = sceClibStrnlen(metadata.artist, 0x3F);
			if (metadata.artist[strlen - 1] == '\n')
				metadata.artist[strlen - 1] = ' ';
			sceClibSnprintf(time, 0x3F, "%s | %d/%d | %s | %s", metadata.artist, selection + 1, count, position_time, length_time);
		}
		else
			sceClibSnprintf(time, 0x3F, "%d/%d | %s | %s", selection + 1, count, position_time, length_time);
	}
	else
		sceClibSnprintf(time, 0x3F, "%d/%d | %s | %s", selection + 1, count, position_time, length_time);

	Utils_Utf8ToUtf16(update_param.notificationSubText, time);

	update_param.targetProgress = ((float)position / (float)length) * 100;
	Utils_NotificationProgressUpdate(&update_param);
}

static void Menu_InitMusic(char *path) {
	Audio_Init(path);

	Menu_LoadExternalCover();

	if (Utils_IsDecoderUsed())
		sceAudioOutSetAlcMode(config.alc_mode);
	else
		shellAudioSetALCModeForMusicPlayer(config.alc_mode);

	if (Utils_IsDecoderUsed())
		sceAudioOutSetEffectType(config.eq_mode);
	else
		shellAudioSetEQModeForMusicPlayer(config.eq_mode);

	filename = malloc(128);
	sceClibSnprintf(filename, 128, Utils_Basename(path));
	position_time = malloc(35);
	length_time = malloc(35);
	length_time_width = 0;

	length = Audio_GetLengthSeconds();
	Menu_ConvertSecondsToString(length_time, length);
	length_time_width = vita2d_pvf_text_width(font, 1, length_time);
	selection = Music_GetCurrentIndex(path);

	if (isFirstTimeInit) {
		initial_selection = selection;
		isFirstTimeInit = SCE_FALSE;
	}

	if (config.notify_mode > 0 && !Utils_IsFinishedPlaylist())
		Menu_NotificationBegin();

	sceClibSnprintf(order, 10, "%d/%d", selection + 1, count);
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
		sceKernelGetRandomNumber(&selection, 4);
		selection = selection % (count - 1);

		if (selection == old_selection)
			selection--;
	}

	Utils_SetMax(&selection, 0, (count - 1));
	Utils_SetMin(&selection, (count - 1), 0);

	if (selection == initial_selection && repeat_check)
		sceKernelSetEventFlag(event_flag_uid, FLAG_ELEVENMPVA_IS_FINISHED_PLAYLIST);

	Audio_Stop();

	free(filename);
	free(length_time);
	free(position_time);

	if ((metadata.has_meta) && (metadata.cover_image) && !ext_cover_loaded) {
		vita2d_wait_rendering_done();
		vita2d_free_texture(metadata.cover_image);
		tex_filter_on = SCE_FALSE;
	}

	Audio_Term();

	/*if (metadata.has_meta && (external_cover[0] != '\0')) {
		empty_metadata.cover_image = metadata.cover_image;
		empty_metadata.has_meta = SCE_TRUE;
	}*/
	// Clear metadata struct
	sceClibMemcpy(&metadata, &empty_metadata, sizeof(Audio_Metadata));
	//sceClibMemset(&empty_metadata, 0, sizeof(Audio_Metadata));

	Menu_InitMusic(playlist[selection]);
}

void Menu_PlayAudio(char *path) {

	Textures_UnloadUnused();

	sceAppMgrAcquireBgmPortForMusicPlayer();

	sceClibMemset(&order, 0, 10);

	Menu_GetMusicList();
	Menu_InitMusic(path);

	sceKernelSetEventFlag(event_flag_uid, FLAG_ELEVENMPVA_IS_POWER_LOCKED);
	vita2d_set_clear_color(RGBA8(53, 63, 73, 255));

	int seek_detect = -1, max_items = 5;
	SceBool isInSettings = SCE_FALSE;
	SceUInt32 pof_timer = 0;

	if (vshSblAimgrIsDolce())
		max_items = 4;

	const char *menu_items[] = {
		"EQ: Off",
		"EQ: Heavy",
		"EQ: Pop",
		"EQ: Jazz",
		"EQ: Unique",
		"Motion Controls"
	};

	Motion_SetState(config.motion_mode);
	Motion_SetReleaseTimer(1000000 * config.motion_timer);
	Motion_SetAngleThreshold(config.motion_degree);

	Menu_LoadExternalCover();

	while(SCE_TRUE) {
		if (!sceKernelPollEventFlag(event_flag_uid, FLAG_ELEVENMPVA_IS_FG, SCE_KERNEL_EVF_WAITMODE_AND, NULL) && !isInSettings) {

			if (!tex_filter_on) {
				if ((metadata.has_meta) && (metadata.cover_image))
					vita2d_texture_set_filters(metadata.cover_image, SCE_GXM_TEXTURE_FILTER_LINEAR, SCE_GXM_TEXTURE_FILTER_LINEAR);
				tex_filter_on = SCE_TRUE;
			}

			vita2d_start_drawing();
			vita2d_clear_screen();

			vita2d_draw_texture(icon_back, BTN_BACK_X, BTN_TOPBAR_Y + 4);

			vita2d_draw_fill_circle(BTN_SETTINGS_X, BTN_TOPBAR_Y + 9, 3, RGBA8(255, 255, 255, 255));
			vita2d_draw_fill_circle(BTN_SETTINGS_X, BTN_TOPBAR_Y + 24, 3, RGBA8(255, 255, 255, 255));
			vita2d_draw_fill_circle(BTN_SETTINGS_X, BTN_TOPBAR_Y + 39, 3, RGBA8(255, 255, 255, 255));

			vita2d_draw_rectangle(410, 124, 550, 400, RGBA8(45, 48, 50, 255)); // Draw info box

#ifdef DEBUG
			vita2d_pvf_draw_textf(font, 50, 100, RGBA8(255, 0, 0, 255), 1, "DEBUG BUILD, DO NOT REDISTRIBUTE. LPDDR2 BUDGET: %f MB", budget_info.freeLPDDR2 / 1024.0 / 1024.0);
#endif

			if ((metadata.has_meta) && (metadata.cover_image))
				vita2d_draw_texture_scale(metadata.cover_image, 0, 124, 400.0f / vita2d_texture_get_width(metadata.cover_image), 400.0f / vita2d_texture_get_height(metadata.cover_image));
			else {
				vita2d_draw_rectangle(0, 124, 400, 400, RGBA8(71, 71, 71, 255));
				vita2d_draw_fill_circle(NOCOVER_X, NOCOVER_Y, 80, RGBA8(45, 48, 50, 255));
				vita2d_draw_fill_circle(NOCOVER_X, NOCOVER_Y, 20, RGBA8(71, 71, 71, 255));
			}

			if ((metadata.has_meta) && (metadata.title[0] != '\0') && (metadata.artist[0] != '\0')) {
				vita2d_pvf_draw_text(font, METADATA1_X, METADATA1_Y, RGBA8(255, 255, 255, 255), 1, metadata.title);
				vita2d_pvf_draw_text(font, METADATA1_X, METADATA1_Y + 30, RGBA8(255, 255, 255, 255), 1, metadata.artist);
			}
			else if ((metadata.has_meta) && (metadata.title[0] != '\0'))
				vita2d_pvf_draw_text(font, METADATA1_X, METADATA1_Y, RGBA8(255, 255, 255, 255), 1, metadata.title);
			else
				vita2d_pvf_draw_text(font, METADATA1_X, METADATA1_Y, RGBA8(255, 255, 255, 255), 1, filename);

			if ((metadata.has_meta) && (metadata.album[0] != '\0'))
				vita2d_pvf_draw_textf(font, METADATA2_X, METADATA2_Y, RGBA8(255, 255, 255, 255), 1, "Album: %s\n", metadata.album);

			if ((metadata.has_meta) && (metadata.year[0] != '\0'))
				vita2d_pvf_draw_textf(font, METADATA2_X, METADATA2_Y + 30, RGBA8(255, 255, 255, 255), 1, "Year: %s\n", metadata.year);

			if ((metadata.has_meta) && (metadata.genre[0] != '\0'))
				vita2d_pvf_draw_textf(font, METADATA2_X, METADATA2_Y + 60, RGBA8(255, 255, 255, 255), 1, "Genre: %s\n", metadata.genre);

			vita2d_pvf_draw_text(font, 910 - vita2d_pvf_text_width(font, 1, order), 446, RGBA8(255, 255, 255, 255), 1, order);
			
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
			vita2d_pvf_draw_text(font, SEEK_X, 480, RGBA8(255, 255, 255, 255), 1, position_time);
			vita2d_pvf_draw_text(font, 910 - length_time_width, 480, RGBA8(255, 255, 255, 255), 1, length_time);
			vita2d_draw_rectangle(SEEK_X, 490, SEEK_WIDTH, 4, RGBA8(71, 71, 71, 255));
			vita2d_draw_rectangle(SEEK_X, 490, (((double)Audio_GetPosition() / (double)Audio_GetLength()) * SEEK_WIDTH_FLOAT), 4, RGBA8(255, 255, 255, 255));

			vita2d_end_drawing();
			vita2d_wait_rendering_done();
			vita2d_end_shfb();

			Touch_Update();

			if (pressed & SCE_CTRL_ENTER || Touch_GetTapRecState(TOUCHREC_TAP_PLAY)) {
				pof_timer = sceKernelGetProcessTimeLow();
				Audio_Pause();
			}

			seek_detect = Touch_GetDragRecStateXPos(TOUCHREC_DRAG_SEEK);

			if (seek_detect > -1) {
				Touch_ChangeRecRectangle(TOUCHREC_DRAG_SEEK, SEEK_X * 2, SEEK_WIDTH * 2, 0, 1088);
				Audio_SetSeekMode(SCE_TRUE);
				Audio_SetSeekPosition(seek_detect - SEEK_X);
			}

			if (seek_detect == -1 && Audio_GetSeekMode()) {
				// Pause first if not paused.
				if (!Audio_IsPaused() && Utils_IsDecoderUsed())
					Audio_Pause();

				Audio_Seek();

				// Unpause.
				if (Audio_IsPaused() && Utils_IsDecoderUsed())
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

			if ((pressed & SCE_CTRL_SELECT) || Touch_GetTapRecState(TOUCHREC_TAP_SETTINGS)) {
				vita2d_set_clear_color(RGBA8(250, 250, 250, 255));
				isInSettings = SCE_TRUE;
			}
		}
		else if (!sceKernelPollEventFlag(event_flag_uid, FLAG_ELEVENMPVA_IS_FG, SCE_KERNEL_EVF_WAITMODE_AND, NULL) && isInSettings) {

			vita2d_start_drawing();
			vita2d_clear_screen();

			vita2d_draw_rectangle(0, 0, 960, 112, RGBA8(51, 51, 51, 255));

			vita2d_draw_texture(icon_back, BTN_BACK_X, BTN_TOPBAR_Y);
			vita2d_pvf_draw_text(font, 102, 40 + ((72 - vita2d_pvf_text_height(font, 1, "Settings")) / 2) + 20, RGBA8(255, 255, 255, 255), 1, "Settings");

			int printed = 0;

			for (int i = 0; i < max_items + 1; i++) {
				if (printed == FILES_PER_PAGE)
					break;

				if (selection < FILES_PER_PAGE || i >(selection - FILES_PER_PAGE)) {
					if (i == selection)
						vita2d_draw_rectangle(0, 112 + (72 * printed), 960, 72, RGBA8(230, 230, 230, 255));

					vita2d_pvf_draw_text(font, 30, 120 + (72 / 2) + (72 * printed), RGBA8(51, 51, 51, 255), 1, menu_items[i]);

					printed++;
				}
			}

			vita2d_draw_texture(config.eq_mode == 0 ? radio_on : radio_off, 850, 126);
			vita2d_draw_texture(config.eq_mode == 1 ? radio_on : radio_off, 850, 198);
			vita2d_draw_texture(config.eq_mode == 2 ? radio_on : radio_off, 850, 270);
			vita2d_draw_texture(config.eq_mode == 3 ? radio_on : radio_off, 850, 342);
			vita2d_draw_texture(config.eq_mode == 4 ? radio_on : radio_off, 850, 414);

			if (!vshSblAimgrIsDolce())
				vita2d_draw_texture(config.motion_mode == SCE_TRUE ? toggle_on : toggle_off, 850, 486);

			vita2d_end_drawing();
			vita2d_wait_rendering_done();
			vita2d_end_shfb();

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
				if (selection != 5) {
					config.eq_mode = selection;

					if (Utils_IsDecoderUsed())
						sceAudioOutSetEffectType(config.eq_mode);
					else
						shellAudioSetEQModeForMusicPlayer(config.eq_mode);

					Dirbrowse_PopulateFiles(SCE_TRUE);
				}
				else {
					config.motion_mode = !config.motion_mode;
					Motion_SetState(config.motion_mode);
				}
			}
		}
		else {
			if (!Utils_IsDecoderUsed() && !config.notify_mode)
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

		if (config.motion_mode) {
			switch (Motion_GetCommand()) {
			case MOTION_NEXT:
				Music_HandleNext(SCE_TRUE, MUSIC_STATE_REPEAT_ALL);
				break;
			case MOTION_PREVIOUS:
				Music_HandleNext(SCE_FALSE, MUSIC_STATE_REPEAT_ALL);
				break;
			case MOTION_STOP:
				pof_timer = sceKernelGetProcessTimeLow();
				Audio_Pause();
				break;
			}
		}

		if (config.notify_mode > 0)
			Menu_NotificationUpdate();

		if (Utils_IsFinishedPlaylist())
			break;

		if (paused && config.power_saving)
			if ((sceKernelGetProcessTimeLow() - pof_timer) > 60000000 * config.power_timer)
				break;
	}

	if (config.notify_mode > 0) {
		Utils_NotificationEnd();
	}

	free(filename);
	free(length_time);
	free(position_time);

	Menu_UnloadExternalCover();
	ext_cover_is_jpeg = SCE_FALSE;

	if ((metadata.has_meta) && (metadata.cover_image)) {
		vita2d_wait_rendering_done();
		vita2d_free_texture(metadata.cover_image);
		tex_filter_on = SCE_FALSE;
	}
	sceClibMemset(&external_cover, 0, 512);

	Audio_Stop();
	Audio_Term();

	// Clear metadata struct
	sceClibMemcpy(&metadata, &empty_metadata, sizeof(Audio_Metadata));

	isFirstTimeInit = SCE_TRUE;
	count = 0;
	initial_selection = 0;
	playing = SCE_FALSE;
	sceAppMgrReleaseBgmPort();
	sceKernelClearEventFlag(event_flag_uid, ~FLAG_ELEVENMPVA_IS_FINISHED_PLAYLIST);
	sceKernelClearEventFlag(event_flag_uid, ~FLAG_ELEVENMPVA_IS_POWER_LOCKED);
	vita2d_set_vblank_wait(0);

	Textures_LoadUnused();

	Menu_DisplayFiles();
}
