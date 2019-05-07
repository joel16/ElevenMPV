#include <string.h>

#include "common.h"
#include "dirbrowse.h"
#include "menu_settings.h"
#include "status_bar.h"
#include "textures.h"
#include "utils.h"

static void Menu_HandleControls(void) {
	if (file_count > 0) {
		if (pressed & SCE_CTRL_UP)
			position--;
		else if (pressed & SCE_CTRL_DOWN)
			position++;

		Utils_SetMax(&position, 0, file_count - 1);
		Utils_SetMin(&position, file_count - 1, 0);

		if (pressed & SCE_CTRL_LEFT)
			position = 0;
		else if (pressed & SCE_CTRL_RIGHT)
			position = file_count - 1;

		if (pressed & SCE_CTRL_ENTER)
			Dirbrowse_OpenFile();
	}

	if ((strcmp(cwd, ROOT_PATH) != 0) && (pressed & SCE_CTRL_CANCEL)) {
		Dirbrowse_Navigate(SCE_TRUE);
		Dirbrowse_PopulateFiles(SCE_TRUE);
	}
}

void Menu_DisplayFiles(void) {
	Dirbrowse_PopulateFiles(SCE_FALSE);
	vita2d_set_clear_color(RGBA8(250, 250, 250, 255));

	while (SCE_TRUE) {
		vita2d_start_drawing();
		vita2d_clear_screen();

		vita2d_draw_rectangle(0, 0, 960, 40, RGBA8(40, 40, 40, 255));
		vita2d_draw_rectangle(0, 40, 960, 72, RGBA8(51, 51, 51, 255));
		StatusBar_Display();
		Dirbrowse_DisplayFiles();

		vita2d_end_drawing();
		vita2d_swap_buffers();

		Utils_ReadControls();
		Menu_HandleControls();

		if (pressed & SCE_CTRL_SELECT)
			Menu_DisplaySettings();

		if (pressed & SCE_CTRL_START)
			break;
	}
}
