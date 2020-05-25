#include <psp2/io/fcntl.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/appmgr.h>
#include <string.h>

#include "config.h"
#include "touch.h"
#include "common.h"
#include "dirbrowse.h"
#include "fs.h"
#include "menu_settings.h"
#include "textures.h"
#include "utils.h"

extern int isFG;

static void Menu_DisplayDeviceSettings(void) {
	int selection = 0, max_items = 4;

	const char *menu_items[] = {
		"ux0:/",
		"ur0:/",
		"uma0:/",
		"xmc0:/",
		"grw0:/"
	};

	while (SCE_TRUE) {
		if (isFG) {
			vita2d_start_drawing();
			vita2d_clear_screen();

			vita2d_draw_rectangle(0, 0, 960, 112, RGBA8(51, 51, 51, 255));

			vita2d_draw_texture(icon_back, BTN_BACK_X, BTN_TOPBAR_Y);
			vita2d_font_draw_text(font, 102, 40 + ((72 - vita2d_font_text_height(font, 25, "Device Settings")) / 2) + 20, RGBA8(255, 255, 255, 255), 25, "Device Settings");

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

			vita2d_draw_texture(config.device == 0 ? radio_on : radio_off, 850, 126);
			vita2d_draw_texture(config.device == 1 ? radio_on : radio_off, 850, 198);
			vita2d_draw_texture(config.device == 2 ? radio_on : radio_off, 850, 270);
			vita2d_draw_texture(config.device == 3 ? radio_on : radio_off, 850, 342);
			vita2d_draw_texture(config.device == 4 ? radio_on : radio_off, 850, 414);

			vita2d_end_drawing();
			vita2d_wait_rendering_done();
			vita2d_end_shfb();

			Utils_ReadControls();
			Touch_Update();

			if ((pressed & SCE_CTRL_CANCEL) || Touch_GetTapRecState(TOUCHREC_TAP_BACK))
				break;

			if (pressed & SCE_CTRL_UP) {
				selection--;
			}
			else if (pressed & SCE_CTRL_DOWN)
				selection++;

			Utils_SetMax(&selection, 0, max_items);
			Utils_SetMin(&selection, max_items, 0);

			if (pressed & SCE_CTRL_ENTER) {
				if (FS_DirExists(menu_items[selection])) {
					config.device = selection;
					Config_Save(config);
					strcpy(root_path, menu_items[config.device]);
					strcpy(cwd, root_path);
					sceIoRemove("savedata0:lastdir.txt");
					Dirbrowse_PopulateFiles(SCE_TRUE);
				}
			}
		}
		else {
			sceKernelDelayThread(10 * 1000);
		}

		if (Utils_AppStatusIsRunning())
			break;
	}
}

static void Menu_DisplaySortSettings(void) {
	int selection = 0, max_items = 3;

	const char *menu_items[] = {
		"By name (ascending)",
		"By name (descending)",
		"By size (largest first)",
		"By size (smallest first)"
	};

	while (SCE_TRUE) {
		if (isFG) {
			vita2d_start_drawing();
			vita2d_clear_screen();

			vita2d_draw_rectangle(0, 0, 960, 112, RGBA8(51, 51, 51, 255));

			vita2d_draw_texture(icon_back, BTN_BACK_X, BTN_TOPBAR_Y);
			vita2d_font_draw_text(font, 102, 40 + ((72 - vita2d_font_text_height(font, 25, "Sort Settings")) / 2) + 20, RGBA8(255, 255, 255, 255), 25, "Sort Settings");

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

			vita2d_draw_texture(config.sort == 0 ? radio_on : radio_off, 850, 126);
			vita2d_draw_texture(config.sort == 1 ? radio_on : radio_off, 850, 198);
			vita2d_draw_texture(config.sort == 2 ? radio_on : radio_off, 850, 270);
			vita2d_draw_texture(config.sort == 3 ? radio_on : radio_off, 850, 342);

			vita2d_end_drawing();
			vita2d_wait_rendering_done();
			vita2d_end_shfb();

			Utils_ReadControls();
			Touch_Update();

			if ((pressed & SCE_CTRL_CANCEL) || Touch_GetTapRecState(TOUCHREC_TAP_BACK))
				break;

			if (pressed & SCE_CTRL_UP)
				selection--;
			else if (pressed & SCE_CTRL_DOWN)
				selection++;

			Utils_SetMax(&selection, 0, max_items);
			Utils_SetMin(&selection, max_items, 0);

			if (pressed & SCE_CTRL_ENTER) {
				config.sort = selection;
				Config_Save(config);
				Dirbrowse_PopulateFiles(SCE_TRUE);
			}
		}
		else {
			sceKernelDelayThread(10 * 1000);
		}

		if (Utils_AppStatusIsRunning())
			break;
	}
}

static void Menu_DisplayMetadataSettings(void) {
	int selection = 0, max_items = 2;

	const char *menu_items[] = {
		"Enable FLAC metadata",
		"Enable MP3 metadata",
		"Enable OPUS metadata"
	};

	while (SCE_TRUE) {
		if (isFG) {
			vita2d_start_drawing();
			vita2d_clear_screen();

			vita2d_draw_rectangle(0, 0, 960, 112, RGBA8(51, 51, 51, 255));

			vita2d_draw_texture(icon_back, BTN_BACK_X, BTN_TOPBAR_Y);
			vita2d_font_draw_text(font, 102, 40 + ((72 - vita2d_font_text_height(font, 25, "Metadata Settings")) / 2) + 20, RGBA8(255, 255, 255, 255), 25, "Metadata Settings");

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

			vita2d_draw_texture(config.meta_flac == SCE_TRUE ? toggle_on : toggle_off, 850, 118);
			vita2d_draw_texture(config.meta_mp3 == SCE_TRUE ? toggle_on : toggle_off, 850, 190);
			vita2d_draw_texture(config.meta_opus == SCE_TRUE ? toggle_on : toggle_off, 850, 262);

			vita2d_end_drawing();
			vita2d_wait_rendering_done();
			vita2d_end_shfb();

			Utils_ReadControls();
			Touch_Update();

			if ((pressed & SCE_CTRL_CANCEL) || Touch_GetTapRecState(TOUCHREC_TAP_BACK))
				break;

			if (pressed & SCE_CTRL_UP)
				selection--;
			else if (pressed & SCE_CTRL_DOWN)
				selection++;

			Utils_SetMax(&selection, 0, max_items);
			Utils_SetMin(&selection, max_items, 0);

			if (pressed & SCE_CTRL_ENTER) {
				switch (selection) {
				case 0:
					config.meta_flac = !config.meta_flac;
					Config_Save(config);
					break;

				case 1:
					config.meta_mp3 = !config.meta_mp3;
					Config_Save(config);
					break;

				case 2:
					config.meta_opus = !config.meta_opus;
					Config_Save(config);
					break;
				}
			}
		}
		else {
			sceKernelDelayThread(10 * 1000);
		}

		if (Utils_AppStatusIsRunning())
			break;
	}
}

static void Menu_DisplayALCModeSettings(void) {
	while (SCE_TRUE) {
		if (isFG) {
			vita2d_start_drawing();
			vita2d_clear_screen();

			vita2d_draw_rectangle(0, 0, 960, 112, RGBA8(51, 51, 51, 255));

			vita2d_draw_texture(icon_back, BTN_BACK_X, BTN_TOPBAR_Y);
			vita2d_font_draw_text(font, 102, 40 + ((72 - vita2d_font_text_height(font, 25, "Dynamic Normalizer")) / 2) + 20, RGBA8(255, 255, 255, 255), 25, "Dynamic Normalizer");

			vita2d_draw_rectangle(0, 112, 960, 72, RGBA8(230, 230, 230, 255));

			vita2d_font_draw_text(font, 30, 120 + (72 / 2), RGBA8(51, 51, 51, 255), 25, "ALC");

			vita2d_draw_texture(config.alc_mode == SCE_TRUE ? toggle_on : toggle_off, 850, 118);

			vita2d_end_drawing();
			vita2d_wait_rendering_done();
			vita2d_end_shfb();

			Utils_ReadControls();
			Touch_Update();

			if ((pressed & SCE_CTRL_CANCEL) || Touch_GetTapRecState(TOUCHREC_TAP_BACK))
				break;

			if (pressed & SCE_CTRL_ENTER) {
				config.alc_mode = !config.alc_mode;
				Config_Save(config);
			}
		}
		else {
			sceKernelDelayThread(10 * 1000);
		}

		if (Utils_AppStatusIsRunning())
			break;
	}
}

static void Menu_DisplayEQSettings(void) {
	int selection = 0, max_items = 4;

	const char *menu_items[] = {
		"Off",
		"Heavy",
		"Pop",
		"Jazz",
		"Unique"
	};

	while (SCE_TRUE) {
		if (isFG) {
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

			if ((pressed & SCE_CTRL_CANCEL) || Touch_GetTapRecState(TOUCHREC_TAP_BACK))
				break;

			if (pressed & SCE_CTRL_UP)
				selection--;
			else if (pressed & SCE_CTRL_DOWN)
				selection++;

			Utils_SetMax(&selection, 0, max_items);
			Utils_SetMin(&selection, max_items, 0);

			if (pressed & SCE_CTRL_ENTER) {
				config.eq_mode = selection;
				Config_Save(config);
				Dirbrowse_PopulateFiles(SCE_TRUE);
			}
		}
		else {
			sceKernelDelayThread(10 * 1000);
		}

		if (Utils_AppStatusIsRunning())
			break;
	}
}

void Menu_DisplaySettings(void) {
	int selection = 0, max_items = 4;

	const char *menu_items[] = {
		"Device Settings",
		"Sort Settings",
		"Metadata Settings",
		"Dynamic Normalizer",
		"Equalizer"
	};

	while (SCE_TRUE) {
		if (isFG) {
			vita2d_start_drawing();
			vita2d_clear_screen();

			vita2d_draw_rectangle(0, 0, 960, 112, RGBA8(51, 51, 51, 255));

			vita2d_draw_texture(icon_back, BTN_BACK_X, BTN_TOPBAR_Y);
			vita2d_font_draw_text(font, 102, 40 + ((72 - vita2d_font_text_height(font, 25, "Settings")) / 2) + 20, RGBA8(255, 255, 255, 255), 25, "Settings");

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
			vita2d_end_drawing();
			vita2d_wait_rendering_done();
			vita2d_end_shfb();

			Utils_ReadControls();
			Touch_Update();

			if ((pressed & SCE_CTRL_CANCEL) || Touch_GetTapRecState(TOUCHREC_TAP_BACK))
				break;

			if (pressed & SCE_CTRL_UP)
				selection--;
			else if (pressed & SCE_CTRL_DOWN)
				selection++;

			Utils_SetMax(&selection, 0, max_items);
			Utils_SetMin(&selection, max_items, 0);

			if (pressed & SCE_CTRL_ENTER) {
				switch (selection) {
				case 0:
					Menu_DisplayDeviceSettings();
					break;
				case 1:
					Menu_DisplaySortSettings();
					break;
				case 2:
					Menu_DisplayMetadataSettings();
					break;
				case 3:
					Menu_DisplayALCModeSettings();
					break;
				case 4:
					Menu_DisplayEQSettings();
					break;
				}
			}
		}
		else {
			sceKernelDelayThread(10 * 1000);
		}

		if (Utils_AppStatusIsRunning())
			break;
	}
}
