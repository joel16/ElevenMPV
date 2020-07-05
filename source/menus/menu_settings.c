#include <psp2/io/fcntl.h>
#include <psp2/kernel/clib.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/vshbridge.h>
#include <psp2/libc.h>
#include <psp2/appmgr.h>
#include <psp2/ime.h>

#include "config.h"
#include "touch.h"
#include "motion.h"
#include "common.h"
#include "dirbrowse.h"
#include "fs.h"
#include "menu_settings.h"
#include "textures.h"
#include "utils.h"

extern SceUID event_flag_uid;

static SceBool ime_active = SCE_FALSE;
static char power_timer_outval[SCE_IME_MAX_PREEDIT_LENGTH * 2 + 4];
static char motion_timer_outval[SCE_IME_MAX_PREEDIT_LENGTH * 2 + 4];
static char motion_degree_outval[SCE_IME_MAX_PREEDIT_LENGTH * 2 + 6];
static char motion_degree_outval_utf8[3];

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
		sceKernelWaitEventFlag(event_flag_uid, FLAG_ELEVENMPVA_IS_FG, SCE_KERNEL_EVF_WAITMODE_AND, NULL, NULL);

        vita2d_start_drawing();
        vita2d_clear_screen();

        vita2d_draw_rectangle(0, 0, 960, 112, RGBA8(51, 51, 51, 255));

        vita2d_draw_texture(icon_back, BTN_BACK_X, BTN_TOPBAR_Y);
        vita2d_pvf_draw_text(font, 102, 40 + ((72 - vita2d_pvf_text_height(font, 1, "Device")) / 2) + 20, RGBA8(255, 255, 255, 255), 1, "Device");

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

        vita2d_draw_texture(config.device == 0 ? radio_on : radio_off, 850, 126);
        vita2d_draw_texture(config.device == 1 ? radio_on : radio_off, 850, 198);
        vita2d_draw_texture(config.device == 2 ? radio_on : radio_off, 850, 270);
        vita2d_draw_texture(config.device == 3 ? radio_on : radio_off, 850, 342);
        vita2d_draw_texture(config.device == 4 ? radio_on : radio_off, 850, 414);

        vita2d_end_drawing();
        vita2d_wait_rendering_done();
        vita2d_end_shfb();

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
                sceLibcStrcpy(root_path, menu_items[config.device]);
				sceLibcStrcpy(cwd, root_path);
                sceIoRemove("savedata0:lastdir.txt");
                Dirbrowse_PopulateFiles(SCE_TRUE);
            }
        }
	}
}

static void Menu_DisplaySortSettings(void) {
	int selection = 0, max_items = 3;

	const char *menu_items[] = {
		"By Name (Ascending)",
		"By Name (Descending)",
		"By Size (Largest First)",
		"By Size (Smallest First)"
	};

	while (SCE_TRUE) {
		sceKernelWaitEventFlag(event_flag_uid, FLAG_ELEVENMPVA_IS_FG, SCE_KERNEL_EVF_WAITMODE_AND, NULL, NULL);

		vita2d_start_drawing();
		vita2d_clear_screen();

		vita2d_draw_rectangle(0, 0, 960, 112, RGBA8(51, 51, 51, 255));

		vita2d_draw_texture(icon_back, BTN_BACK_X, BTN_TOPBAR_Y);
		vita2d_pvf_draw_text(font, 102, 40 + ((72 - vita2d_pvf_text_height(font, 1, "Sort")) / 2) + 20, RGBA8(255, 255, 255, 255), 1, "Sort");

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

		vita2d_draw_texture(config.sort == 0 ? radio_on : radio_off, 850, 126);
		vita2d_draw_texture(config.sort == 1 ? radio_on : radio_off, 850, 198);
		vita2d_draw_texture(config.sort == 2 ? radio_on : radio_off, 850, 270);
		vita2d_draw_texture(config.sort == 3 ? radio_on : radio_off, 850, 342);

		vita2d_end_drawing();
		vita2d_wait_rendering_done();
		vita2d_end_shfb();

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
			Dirbrowse_PopulateFiles(SCE_TRUE);
		}
	}
}

static void Menu_DisplayAudioSettings(void) {
	int selection = 0, max_items = 6;

	const char *menu_items[] = {
		"EQ: Off",
		"EQ: Heavy",
		"EQ: Pop",
		"EQ: Jazz",
		"EQ: Unique",
		"ALC",
		"Limit Volume Whith EQ"
	};

	while (SCE_TRUE) {
		sceKernelWaitEventFlag(event_flag_uid, FLAG_ELEVENMPVA_IS_FG, SCE_KERNEL_EVF_WAITMODE_AND, NULL, NULL);

		vita2d_start_drawing();
		vita2d_clear_screen();

		vita2d_draw_rectangle(0, 0, 960, 112, RGBA8(51, 51, 51, 255));

		vita2d_draw_texture(icon_back, BTN_BACK_X, BTN_TOPBAR_Y);
		vita2d_pvf_draw_text(font, 102, 40 + ((72 - vita2d_pvf_text_height(font, 1, "Audio")) / 2) + 20, RGBA8(255, 255, 255, 255), 1, "Audio");

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

		if (selection != 6) {
			vita2d_draw_texture(config.eq_mode == 0 ? radio_on : radio_off, 850, 126);
			vita2d_draw_texture(config.eq_mode == 1 ? radio_on : radio_off, 850, 198);
			vita2d_draw_texture(config.eq_mode == 2 ? radio_on : radio_off, 850, 270);
			vita2d_draw_texture(config.eq_mode == 3 ? radio_on : radio_off, 850, 342);
			vita2d_draw_texture(config.eq_mode == 4 ? radio_on : radio_off, 850, 414);
			vita2d_draw_texture(config.alc_mode == SCE_TRUE ? toggle_on : toggle_off, 850, 486);
		}
		else {
			vita2d_draw_texture(config.eq_mode == 1 ? radio_on : radio_off, 850, 126);
			vita2d_draw_texture(config.eq_mode == 2 ? radio_on : radio_off, 850, 198);
			vita2d_draw_texture(config.eq_mode == 3 ? radio_on : radio_off, 850, 270);
			vita2d_draw_texture(config.eq_mode == 4 ? radio_on : radio_off, 850, 342);
			vita2d_draw_texture(config.alc_mode == SCE_TRUE ? toggle_on : toggle_off, 850, 414);
			vita2d_draw_texture(config.eq_volume == SCE_TRUE ? toggle_on : toggle_off, 850, 486);
		}


		vita2d_end_drawing();
		vita2d_wait_rendering_done();
		vita2d_end_shfb();

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
			if (selection < 5) {
				config.eq_mode = selection;
				Dirbrowse_PopulateFiles(SCE_TRUE);
			}
			else if (selection == 5) {
				config.alc_mode = !config.alc_mode;
			}
			else if (selection == 6) {
				config.eq_volume = !config.eq_volume;
			}
		}
	}
}

void Menu_OnImeEvent(void *arg, const SceImeEvent *e)
{
	switch (e->id) {
	case SCE_IME_EVENT_PRESS_ENTER:
		ime_active = SCE_FALSE;
		switch (*(int *)arg) {
		case 3:
			if (power_timer_outval[0] == '\0' || power_timer_outval[0] == '0')
				power_timer_outval[0] = '1';
			config.power_timer = power_timer_outval[0] - '0';
			break;
		case 1:
			if (motion_timer_outval[0] == '\0' || motion_timer_outval[0] == '0')
				motion_timer_outval[0] = '1';
			config.motion_timer = motion_timer_outval[0] - '0';
			break;
		case 2:
			if (motion_degree_outval_utf8[0] == '\0' || motion_degree_outval_utf8[0] == '0')
				motion_degree_outval_utf8[0] = '1';
			config.motion_degree = sceLibcAtoi(motion_degree_outval_utf8);
			break;
		}
		sceImeClose();
		break;
	case SCE_IME_EVENT_PRESS_CLOSE:
		ime_active = SCE_FALSE;
		switch (*(int *)arg) {
		case 1:
			motion_timer_outval[0] = config.motion_timer + '0';
			break;
		case 2:
			sceClibSnprintf(motion_degree_outval_utf8, 2, "%d", config.motion_degree);
			break;
		case 3:
			power_timer_outval[0] = config.power_timer + '0';
			break;
		}
		sceImeClose();
		break;
	}
}

static void Menu_DisplayControlsSettings(void) {
	int selection = 0, max_items = 2, type = 0;

	char ime_initval[4];
	sceClibMemset(&motion_timer_outval, 0, (SCE_IME_MAX_PREEDIT_LENGTH * 2 + 4));
	sceClibMemset(&motion_degree_outval, 0, (SCE_IME_MAX_PREEDIT_LENGTH * 2 + 6));
	sceClibMemset(&ime_initval, 0, 4);
	motion_timer_outval[0] = config.motion_timer + '0';
	sceClibSnprintf(motion_degree_outval_utf8, 2, "%d", config.motion_degree);
	motion_degree_outval_utf8[2] = '\0';
	SceImeParam param;

	const char *menu_items[] = {
		"Motion Controls",
		"Timeout                                                                                                         %s sec",
		"Angle Threshold                                                                                               %s °"
	};

	while (SCE_TRUE) {
		sceKernelWaitEventFlag(event_flag_uid, FLAG_ELEVENMPVA_IS_FG, SCE_KERNEL_EVF_WAITMODE_AND, NULL, NULL);

		vita2d_start_drawing();
		vita2d_clear_screen();

		vita2d_draw_rectangle(0, 0, 960, 112, RGBA8(51, 51, 51, 255));

		vita2d_draw_texture(icon_back, BTN_BACK_X, BTN_TOPBAR_Y);
		vita2d_pvf_draw_text(font, 102, 40 + ((72 - vita2d_pvf_text_height(font, 1, "Motion Controls")) / 2) + 20, RGBA8(255, 255, 255, 255), 1, "Motion Controls");

		int printed = 0;

		if (!ime_active) {
			for (int i = 0; i < max_items + 1; i++) {
				if (printed == FILES_PER_PAGE)
					break;

				if (selection < FILES_PER_PAGE || i >(selection - FILES_PER_PAGE)) {
					if (i == selection)
						vita2d_draw_rectangle(0, 112 + (72 * printed), 960, 72, RGBA8(230, 230, 230, 255));

					switch (i) {
					case 1:
						vita2d_pvf_draw_textf(font, 30, 120 + (72 / 2) + (72 * printed), RGBA8(51, 51, 51, 255), 1, menu_items[i], motion_timer_outval);
						break;
					case 2:
						vita2d_pvf_draw_textf(font, 30, 120 + (72 / 2) + (72 * printed), RGBA8(51, 51, 51, 255), 1, menu_items[i], motion_degree_outval_utf8);
						break;
					default:
						vita2d_pvf_draw_text(font, 30, 120 + (72 / 2) + (72 * printed), RGBA8(51, 51, 51, 255), 1, menu_items[i]);
						break;
					}

					printed++;
				}
			}
			vita2d_draw_texture(config.motion_mode == SCE_TRUE ? toggle_on : toggle_off, 850, 118);
		}
		else {
			vita2d_draw_rectangle(0, 112 + (72 * printed), 960, 72, RGBA8(230, 230, 230, 255));
			switch (selection) {
			case 1:
				vita2d_pvf_draw_textf(font, 30, 120 + (72 / 2) + (72 * printed), RGBA8(51, 51, 51, 255), 1, menu_items[1], motion_timer_outval);
				break;
			case 2:
				vita2d_pvf_draw_textf(font, 30, 120 + (72 / 2) + (72 * printed), RGBA8(51, 51, 51, 255), 1, menu_items[2], motion_degree_outval_utf8);
				break;
			}
		}

		vita2d_end_drawing();
		vita2d_wait_rendering_done();
		vita2d_end_shfb();

		if (!ime_active) {

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
					config.motion_mode = !config.motion_mode;
					break;

				case 1:
					Utils_LoadIme(&param);
					type = 1;
					param.arg = &type;
					param.type = SCE_IME_TYPE_NUMBER;
					param.inputTextBuffer = (SceWChar16 *)motion_timer_outval;
					param.maxTextLength = 1;
					param.handler = Menu_OnImeEvent;
					param.initialText = (SceWChar16 *)ime_initval;
					sceImeOpen(&param);
					ime_active = SCE_TRUE;
					break;
				case 2:
					Utils_LoadIme(&param);
					type = 2;
					param.arg = &type;
					param.type = SCE_IME_TYPE_NUMBER;
					param.inputTextBuffer = (SceWChar16 *)motion_degree_outval;
					param.maxTextLength = 2;
					param.handler = Menu_OnImeEvent;
					param.initialText = (SceWChar16 *)ime_initval;
					sceImeOpen(&param);
					ime_active = SCE_TRUE;
					break;
				}
			}
		}
		else {
			sceImeUpdate();
			if (selection == 2 && ime_active) {
				motion_degree_outval_utf8[0] = motion_degree_outval[0];
				motion_degree_outval_utf8[1] = motion_degree_outval[2];
			}
		}
	}

	Utils_UnloadIme();
}

static void Menu_DisplayPowerSettings(void) {
	int selection = 0, max_items = 1, type = 0;

	char ime_initval[4];
	sceClibMemset(&power_timer_outval, 0, (SCE_IME_MAX_PREEDIT_LENGTH * 2 + 4));
	sceClibMemset(&ime_initval, 0, 4);
	power_timer_outval[0] = config.power_timer + '0';
	SceImeParam param;

	const char *menu_items[] = {
		"Suspend Automatically When Paused",
		"Suspend Timer                                                                                               %s min"
	};

	while (SCE_TRUE) {
		sceKernelWaitEventFlag(event_flag_uid, FLAG_ELEVENMPVA_IS_FG, SCE_KERNEL_EVF_WAITMODE_AND, NULL, NULL);

		vita2d_start_drawing();
		vita2d_clear_screen();

		vita2d_draw_rectangle(0, 0, 960, 112, RGBA8(51, 51, 51, 255));

		vita2d_draw_texture(icon_back, BTN_BACK_X, BTN_TOPBAR_Y);
		vita2d_pvf_draw_text(font, 102, 40 + ((72 - vita2d_pvf_text_height(font, 1, "Power Saving")) / 2) + 20, RGBA8(255, 255, 255, 255), 1, "Power Saving");

		int printed = 0;

		if (!ime_active) {
			for (int i = 0; i < max_items + 1; i++) {
				if (printed == FILES_PER_PAGE)
					break;

				if (selection < FILES_PER_PAGE || i >(selection - FILES_PER_PAGE)) {
					if (i == selection)
						vita2d_draw_rectangle(0, 112 + (72 * printed), 960, 72, RGBA8(230, 230, 230, 255));

					switch (i) {
					case 1:
						vita2d_pvf_draw_textf(font, 30, 120 + (72 / 2) + (72 * printed), RGBA8(51, 51, 51, 255), 1, menu_items[i], power_timer_outval);
						break;
					default:
						vita2d_pvf_draw_text(font, 30, 120 + (72 / 2) + (72 * printed), RGBA8(51, 51, 51, 255), 1, menu_items[i]);
						break;
					}

					printed++;
				}
			}
			vita2d_draw_texture(config.power_saving == SCE_TRUE ? toggle_on : toggle_off, 850, 118);
		}
		else {
			vita2d_draw_rectangle(0, 112 + (72 * printed), 960, 72, RGBA8(230, 230, 230, 255));
			switch (selection) {
			case 1:
				vita2d_pvf_draw_textf(font, 30, 120 + (72 / 2) + (72 * printed), RGBA8(51, 51, 51, 255), 1, menu_items[1], power_timer_outval);
				break;
			}
		}

		vita2d_end_drawing();
		vita2d_wait_rendering_done();
		vita2d_end_shfb();

		if (!ime_active) {

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
					config.power_saving = !config.power_saving;
					break;

				case 1:
					Utils_LoadIme(&param);
					type = 3;
					param.arg = &type;
					param.type = SCE_IME_TYPE_NUMBER;
					param.inputTextBuffer = (SceWChar16 *)power_timer_outval;
					param.maxTextLength = 1;
					param.handler = Menu_OnImeEvent;
					param.initialText = (SceWChar16 *)ime_initval;
					sceImeOpen(&param);
					ime_active = SCE_TRUE;
					break;
				}
			}
		}
		else
			sceImeUpdate();
	}

	Utils_UnloadIme();
}

static void Menu_DisplaySystemSettings(void) {
	int selection = 0, max_items = 2;

	const char *menu_items[] = {
		"Off",
		"On, Cancel Button: Stop",
		"On, Cancel Button: Stop and Exit"
	};

	while (SCE_TRUE) {
		sceKernelWaitEventFlag(event_flag_uid, FLAG_ELEVENMPVA_IS_FG, SCE_KERNEL_EVF_WAITMODE_AND, NULL, NULL);

		vita2d_start_drawing();
		vita2d_clear_screen();

		vita2d_draw_rectangle(0, 0, 960, 112, RGBA8(51, 51, 51, 255));

		vita2d_draw_texture(icon_back, BTN_BACK_X, BTN_TOPBAR_Y);
		vita2d_pvf_draw_text(font, 102, 40 + ((72 - vita2d_pvf_text_height(font, 1, "Notifications")) / 2) + 20, RGBA8(255, 255, 255, 255), 1, "Notifications");

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

		vita2d_draw_texture(config.notify_mode == 0 ? radio_on : radio_off, 850, 126);
		vita2d_draw_texture(config.notify_mode == 1 ? radio_on : radio_off, 850, 198);
		vita2d_draw_texture(config.notify_mode == 2 ? radio_on : radio_off, 850, 270);

		vita2d_end_drawing();
		vita2d_wait_rendering_done();
		vita2d_end_shfb();

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
			config.notify_mode = selection;
			Dirbrowse_PopulateFiles(SCE_TRUE);
		}
	}
}

void Menu_DisplaySettings(void) {
	int selection = 0, max_items = 5;

	if (vshSblAimgrIsDolce())
		max_items = 4;

	const char *menu_items[] = {
		"Device",
		"Sort",
		"Audio",
		"Power Saving",
		"Notifications",
		"Motion Controls"
	};

	while (SCE_TRUE) {
		sceKernelWaitEventFlag(event_flag_uid, FLAG_ELEVENMPVA_IS_FG, SCE_KERNEL_EVF_WAITMODE_AND, NULL, NULL);

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

		vita2d_end_drawing();
		vita2d_wait_rendering_done();
		vita2d_end_shfb();

		Touch_Update();

		if ((pressed & SCE_CTRL_CANCEL) || Touch_GetTapRecState(TOUCHREC_TAP_BACK)) {
			break;
		}

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
				Menu_DisplayAudioSettings();
				break;
			case 3:
				Menu_DisplayPowerSettings();
				break;
			case 4:
				Menu_DisplaySystemSettings();
				break;
			case 5:
				Menu_DisplayControlsSettings();
				break;
			}
		}
	}
}
