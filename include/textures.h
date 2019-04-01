#ifndef _ELEVENMPV_TEXTURES_H_
#define _ELEVENMPV_TEXTURES_H_

#include <vita2d.h>

vita2d_texture *icon_dir, *icon_file, *icon_audio, *battery_20, *battery_20_charging, *battery_30, *battery_30_charging, *battery_50, *battery_50_charging, \
	*battery_60, *battery_60_charging, *battery_80, *battery_80_charging, *battery_90, *battery_90_charging, *battery_full, *battery_full_charging, \
	*battery_low, *battery_unknown, *default_artwork, *default_artwork_blur, *btn_play, *btn_pause, *btn_rewind, *btn_forward, *btn_repeat, \
	*btn_shuffle, *btn_repeat_overlay, *btn_shuffle_overlay, *icon_back;

void Textures_Load(void);
void Textures_Free(void);

#endif
