#include <psp2/io/stat.h> 
#include <psp2/kernel/clib.h>
#include <psp2/io/fcntl.h> 
#include <psp2/sysmodule.h>

#include "textures.h"

void Textures_Load(void) {
	btn_forward = vita2d_load_GXT_file("app0:texture.gxt", 0, 0);
	btn_pause = vita2d_load_additional_GXT(btn_forward, 1);
	btn_play = vita2d_load_additional_GXT(btn_forward, 2);
	btn_repeat = vita2d_load_additional_GXT(btn_forward, 3);
	btn_rewind = vita2d_load_additional_GXT(btn_forward, 4);
	btn_shuffle = vita2d_load_additional_GXT(btn_forward, 5);

	icon_audio = vita2d_load_additional_GXT(btn_forward, 6);
	icon_back = vita2d_load_additional_GXT(btn_forward, 7);
	icon_file = vita2d_load_additional_GXT(btn_forward, 8);
	icon_dir = vita2d_load_additional_GXT(btn_forward, 9);

	radio_on = vita2d_load_additional_GXT(btn_forward, 10);
	radio_off = vita2d_load_additional_GXT(btn_forward, 11);
	toggle_off = vita2d_load_additional_GXT(btn_forward, 12);
	toggle_on = vita2d_load_additional_GXT(btn_forward, 13);
}
