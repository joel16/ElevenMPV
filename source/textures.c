#include <psp2/kernel/clib.h>
#include <psp2/kernel/iofilemgr.h> 
#include <psp2/sysmodule.h>

#include "textures.h"

void Textures_Load(void) {
	btn_forward = vita2d_load_GXT_file("app0:tex_common.gxt", 0, 0);
	btn_pause = vita2d_load_additional_GXT(btn_forward, 1);
	btn_play = vita2d_load_additional_GXT(btn_forward, 2);
	btn_repeat = vita2d_load_additional_GXT(btn_forward, 3);
	btn_rewind = vita2d_load_additional_GXT(btn_forward, 4);
	btn_shuffle = vita2d_load_additional_GXT(btn_forward, 5);

	icon_back = vita2d_load_additional_GXT(btn_forward, 6);

	radio_on = vita2d_load_additional_GXT(btn_forward, 7);
	radio_off = vita2d_load_additional_GXT(btn_forward, 8);
	toggle_off = vita2d_load_additional_GXT(btn_forward, 9);
	toggle_on = vita2d_load_additional_GXT(btn_forward, 10);

	icon_audio = vita2d_load_GXT_file("app0:tex_dirbrowse.gxt", 0, 0);
	icon_file = vita2d_load_additional_GXT(icon_audio, 1);
	icon_dir = vita2d_load_additional_GXT(icon_audio, 2);
}

void Textures_UnloadUnused(void) {
	vita2d_free_additional_GXT(icon_file);
	vita2d_free_additional_GXT(icon_dir);
	vita2d_free_texture(icon_audio);
}

void Textures_LoadUnused(void) {
	icon_audio = vita2d_load_GXT_file("app0:tex_dirbrowse.gxt", 0, 0);
	icon_file = vita2d_load_additional_GXT(icon_audio, 1);
	icon_dir = vita2d_load_additional_GXT(icon_audio, 2);
}


