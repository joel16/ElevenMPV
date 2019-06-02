#include <psp2/kernel/sysmem.h>
#include <string.h>

#include "config.h"
#include "common.h"
#include "dirbrowse.h"
#include "menu_settings.h"
#include "status_bar.h"
#include "textures.h"
#include "utils.h"

static void Menu_DisplayDeviceSettings(void) {
	int is_vita_tv = sceKernelIsPSVitaTV();
	int selection = 0, max_items = is_vita_tv? 2 : 1;

	const char *menu_items[] = {
		"ux0:/",
		"ur0:/",
		"uma0:/"
	};

	while (SCE_TRUE) {
		vita2d_start_drawing();
		vita2d_clear_screen();

		vita2d_draw_rectangle(0, 0, 960, 40, RGBA8(40, 40, 40, 255));
		vita2d_draw_rectangle(0, 40, 960, 72, RGBA8(51, 51, 51, 255));
		StatusBar_Display();

		vita2d_draw_texture(icon_back, 25, 54);
		vita2d_font_draw_text(font, 102, 40 + ((72 - vita2d_font_text_height(font, 25, "Device Settings")) / 2) + 20, RGBA8(255, 255, 255, 255), 25, "Sort Settings");

		int printed = 0;

		for (int i = 0; i < max_items + 1; i++) {
			if (printed == FILES_PER_PAGE)
				break;

			if (selection < FILES_PER_PAGE || i > (selection - FILES_PER_PAGE)) {
				if (i == selection)
					vita2d_draw_rectangle(0, 112 + (72 * printed), 960, 72, RGBA8(230, 230, 230, 255));

				vita2d_font_draw_text(font, 30, 120 + (72 / 2) + (72 * printed), RGBA8(51, 51, 51, 255), 25, menu_items[i]);

				printed++;
			}
		}

		vita2d_draw_texture(config.device == 0? radio_on : radio_off, 850, 126);
		vita2d_draw_texture(config.device == 1? radio_on : radio_off, 850, 198);
		if (is_vita_tv)
			vita2d_draw_texture(config.device == 2? radio_on : radio_off, 850, 270);

		vita2d_end_drawing();
		vita2d_swap_buffers();

		Utils_ReadControls();

		if (pressed & SCE_CTRL_CIRCLE)
			break;

		if (pressed & SCE_CTRL_UP)
			selection--;
		else if (pressed & SCE_CTRL_DOWN)
			selection++;

		Utils_SetMax(&selection, 0, max_items);
		Utils_SetMin(&selection, max_items, 0);

		if (pressed & SCE_CTRL_ENTER) {
			config.device = selection;
			Config_Save(config);
			strcpy(root_path, menu_items[config.device]);
			strcpy(cwd, root_path);
			Dirbrowse_PopulateFiles(SCE_TRUE);
		}
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
		vita2d_start_drawing();
		vita2d_clear_screen();

		vita2d_draw_rectangle(0, 0, 960, 40, RGBA8(40, 40, 40, 255));
		vita2d_draw_rectangle(0, 40, 960, 72, RGBA8(51, 51, 51, 255));
		StatusBar_Display();

		vita2d_draw_texture(icon_back, 25, 54);
		vita2d_font_draw_text(font, 102, 40 + ((72 - vita2d_font_text_height(font, 25, "Sort Settings")) / 2) + 20, RGBA8(255, 255, 255, 255), 25, "Sort Settings");

		int printed = 0;

		for (int i = 0; i < max_items + 1; i++) {
			if (printed == FILES_PER_PAGE)
				break;

			if (selection < FILES_PER_PAGE || i > (selection - FILES_PER_PAGE)) {
				if (i == selection)
					vita2d_draw_rectangle(0, 112 + (72 * printed), 960, 72, RGBA8(230, 230, 230, 255));

				vita2d_font_draw_text(font, 30, 120 + (72 / 2) + (72 * printed), RGBA8(51, 51, 51, 255), 25, menu_items[i]);

				printed++;
			}
		}

		vita2d_draw_texture(config.sort == 0? radio_on : radio_off, 850, 126);
		vita2d_draw_texture(config.sort == 1? radio_on : radio_off, 850, 198);
		vita2d_draw_texture(config.sort == 2? radio_on : radio_off, 850, 270);
		vita2d_draw_texture(config.sort == 3? radio_on : radio_off, 850, 342);

		vita2d_end_drawing();
		vita2d_swap_buffers();

		Utils_ReadControls();

		if (pressed & SCE_CTRL_CIRCLE)
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
}

static void Menu_DisplayMetadataSettings(void) {
	int selection = 0, max_items = 2;

	const char *menu_items[] = {
		"Enable FLAC metadata",
		"Enable MP3 metadata",
		"Enable OPUS metadata"
	};

	while (SCE_TRUE) {
		vita2d_start_drawing();
		vita2d_clear_screen();

		vita2d_draw_rectangle(0, 0, 960, 40, RGBA8(40, 40, 40, 255));
		vita2d_draw_rectangle(0, 40, 960, 72, RGBA8(51, 51, 51, 255));
		StatusBar_Display();

		vita2d_draw_texture(icon_back, 25, 54);
		vita2d_font_draw_text(font, 102, 40 + ((72 - vita2d_font_text_height(font, 25, "Metadata Settings")) / 2) + 20, RGBA8(255, 255, 255, 255), 25, "Metadata Settings");

		int printed = 0;

		for (int i = 0; i < max_items + 1; i++) {
			if (printed == FILES_PER_PAGE)
				break;

			if (selection < FILES_PER_PAGE || i > (selection - FILES_PER_PAGE)) {
				if (i == selection)
					vita2d_draw_rectangle(0, 112 + (72 * printed), 960, 72, RGBA8(230, 230, 230, 255));

				vita2d_font_draw_text(font, 30, 120 + (72 / 2) + (72 * printed), RGBA8(51, 51, 51, 255), 25, menu_items[i]);

				printed++;
			}
		}

		vita2d_draw_texture(config.meta_flac == SCE_TRUE? toggle_on : toggle_off, 850, 118);
		vita2d_draw_texture(config.meta_mp3 == SCE_TRUE? toggle_on : toggle_off, 850, 190);
		vita2d_draw_texture(config.meta_opus == SCE_TRUE? toggle_on : toggle_off, 850, 262);

		vita2d_end_drawing();
		vita2d_swap_buffers();

		Utils_ReadControls();

		if (pressed & SCE_CTRL_CIRCLE)
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
}

static void Menu_DisplayALCModeSettings(void) {
	int selection = 0, max_items = 1;

	const char *menu_items[] = {
		"ALC off",
		"ALC mode 1"
		//"ALC mode max" // Max doesn't seem to work ?
	};

	while (SCE_TRUE) {
		vita2d_start_drawing();
		vita2d_clear_screen();

		vita2d_draw_rectangle(0, 0, 960, 40, RGBA8(40, 40, 40, 255));
		vita2d_draw_rectangle(0, 40, 960, 72, RGBA8(51, 51, 51, 255));
		StatusBar_Display();

		vita2d_draw_texture(icon_back, 25, 54);
		vita2d_font_draw_text(font, 102, 40 + ((72 - vita2d_font_text_height(font, 25, "Dynamic Normalizer")) / 2) + 20, RGBA8(255, 255, 255, 255), 25, "Dynamic Normalizer");

		int printed = 0;

		for (int i = 0; i < max_items + 1; i++) {
			if (printed == FILES_PER_PAGE)
				break;

			if (selection < FILES_PER_PAGE || i > (selection - FILES_PER_PAGE)) {
				if (i == selection)
					vita2d_draw_rectangle(0, 112 + (72 * printed), 960, 72, RGBA8(230, 230, 230, 255));

				vita2d_font_draw_text(font, 30, 120 + (72 / 2) + (72 * printed), RGBA8(51, 51, 51, 255), 25, menu_items[i]);

				printed++;
			}
		}

		vita2d_draw_texture(config.alc_mode == 0? radio_on : radio_off, 850, 126);
		vita2d_draw_texture(config.alc_mode == 1? radio_on : radio_off, 850, 198);

		vita2d_end_drawing();
		vita2d_swap_buffers();

		Utils_ReadControls();

		if (pressed & SCE_CTRL_CIRCLE)
			break;

		if (pressed & SCE_CTRL_UP)
			selection--;
		else if (pressed & SCE_CTRL_DOWN)
			selection++;

		Utils_SetMax(&selection, 0, max_items);
		Utils_SetMin(&selection, max_items, 0);

		if (pressed & SCE_CTRL_ENTER) {
			config.alc_mode = selection;
			Config_Save(config);
			Dirbrowse_PopulateFiles(SCE_TRUE);
		}
	}
}

void Menu_DisplaySettings(void) {
	int selection = 0, max_items = 3;

	const char *menu_items[] = {
		"Device settings",
		"Sort settings",
		"Metadata settings",
		"Dynamic normalizer modes"
	};

	while (SCE_TRUE) {
		vita2d_start_drawing();
		vita2d_clear_screen();

		vita2d_draw_rectangle(0, 0, 960, 40, RGBA8(40, 40, 40, 255));
		vita2d_draw_rectangle(0, 40, 960, 72, RGBA8(51, 51, 51, 255));
		StatusBar_Display();

		vita2d_draw_texture(icon_back, 25, 54);
		vita2d_font_draw_text(font, 102, 40 + ((72 - vita2d_font_text_height(font, 25, "Settings")) / 2) + 20, RGBA8(255, 255, 255, 255), 25, "Settings");

		int printed = 0;

		for (int i = 0; i < max_items + 1; i++) {
			if (printed == FILES_PER_PAGE)
				break;

			if (selection < FILES_PER_PAGE || i > (selection - FILES_PER_PAGE)) {
				if (i == selection)
					vita2d_draw_rectangle(0, 112 + (72 * printed), 960, 72, RGBA8(230, 230, 230, 255));

				vita2d_font_draw_text(font, 30, 120 + (72 / 2) + (72 * printed), RGBA8(51, 51, 51, 255), 25, menu_items[i]);

				printed++;
			}
		}

		vita2d_end_drawing();
		vita2d_swap_buffers();

		Utils_ReadControls();

		if (pressed & SCE_CTRL_CIRCLE)
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
			}
		}
	}
}
