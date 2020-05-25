#include <psp2/kernel/clib.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/appmgr.h>

#include "common.h"
#include "touch.h"
#include "dirbrowse.h"
#include "menu_settings.h"
#include "textures.h"
#include "utils.h"

extern int isFG;

static void Menu_HandleControls(void) {

	Touch_Update();

	if (file_count > 0) {
		if ((pressed & SCE_CTRL_UP) || Touch_GetFileListDragReport() == -1)
			position--;
		else if ((pressed & SCE_CTRL_DOWN) || Touch_GetFileListDragReport() == 1)
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

	if (sceClibStrcmp(cwd, root_path) != 0 && ((pressed & SCE_CTRL_CANCEL) || Touch_GetTapRecState(TOUCHREC_TAP_BACK))) {
		Dirbrowse_Navigate(SCE_TRUE);
		Dirbrowse_PopulateFiles(SCE_TRUE);
	}
}

void Menu_DisplayFiles(void) {
	Dirbrowse_PopulateFiles(SCE_FALSE);
	vita2d_set_clear_color(RGBA8(250, 250, 250, 255));

	while (SCE_TRUE) {
		if (isFG) {
			vita2d_start_drawing();
			vita2d_clear_screen();

			vita2d_draw_rectangle(0, 0, 960, 112, RGBA8(51, 51, 51, 255));

			vita2d_draw_fill_circle(BTN_SETTINGS_X, BTN_TOPBAR_Y + 5, 3, RGBA8(255, 255, 255, 255));
			vita2d_draw_fill_circle(BTN_SETTINGS_X, BTN_TOPBAR_Y + 20, 3, RGBA8(255, 255, 255, 255));
			vita2d_draw_fill_circle(BTN_SETTINGS_X, BTN_TOPBAR_Y + 35, 3, RGBA8(255, 255, 255, 255));

			Dirbrowse_DisplayFiles();

			vita2d_end_drawing();
			vita2d_wait_rendering_done();
			vita2d_end_shfb();

			Utils_ReadControls();
			Menu_HandleControls();

			if ((pressed & SCE_CTRL_SELECT) || Touch_GetTapRecState(TOUCHREC_TAP_SETTINGS))
				Menu_DisplaySettings();

			if (pressed & SCE_CTRL_START)
				break;
		}
		else {
			sceKernelDelayThread(10 * 1000);
		}

		if (Utils_AppStatusIsRunning())
			break;
	}
}
