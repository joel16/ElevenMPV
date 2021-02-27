#ifndef _ELEVENMPV_TEXTURES_H_
#define _ELEVENMPV_TEXTURES_H_

#include <vita2d_sys.h>

#define BTN_TOPBAR_Y 54
#define BTN_BACK_X 10

vita2d_texture *icon_dir, *icon_file, *icon_audio, *btn_play, *btn_pause, *btn_rewind, *btn_forward, *btn_repeat, \
	*btn_shuffle, *icon_back, *toggle_on, *toggle_off, *radio_on, *radio_off, *default_artwork;

void Textures_Load(void);
void Textures_UnloadUnused(void);
void Textures_LoadUnused(void);

#endif
