#include <psp2/types.h>

#include "textures.h"

extern SceUChar8 _binary_res_battery_20_png_start;
extern SceUChar8 _binary_res_battery_30_png_start;
extern SceUChar8 _binary_res_battery_50_png_start;
extern SceUChar8 _binary_res_battery_60_png_start;
extern SceUChar8 _binary_res_battery_80_png_start;
extern SceUChar8 _binary_res_battery_90_png_start;
extern SceUChar8 _binary_res_battery_full_png_start;

extern SceUChar8 _binary_res_battery_20_charging_png_start;
extern SceUChar8 _binary_res_battery_30_charging_png_start;
extern SceUChar8 _binary_res_battery_50_charging_png_start;
extern SceUChar8 _binary_res_battery_60_charging_png_start;
extern SceUChar8 _binary_res_battery_80_charging_png_start;
extern SceUChar8 _binary_res_battery_90_charging_png_start;
extern SceUChar8 _binary_res_battery_full_charging_png_start;

extern SceUChar8 _binary_res_battery_low_png_start;
extern SceUChar8 _binary_res_battery_unknown_png_start;

extern SceUChar8 _binary_res_icon_audio_png_start;
extern SceUChar8 _binary_res_icon_file_png_start;
extern SceUChar8 _binary_res_icon_folder_png_start;
extern SceUChar8 _binary_res_icon_back_png_start;

extern SceUChar8 _binary_res_btn_playback_forward_png_start;
extern SceUChar8 _binary_res_btn_playback_pause_png_start;
extern SceUChar8 _binary_res_btn_playback_play_png_start;
extern SceUChar8 _binary_res_btn_playback_repeat_png_start;
extern SceUChar8 _binary_res_btn_playback_repeat_overlay_png_start;
extern SceUChar8 _binary_res_btn_playback_rewind_png_start;
extern SceUChar8 _binary_res_btn_playback_shuffle_png_start;
extern SceUChar8 _binary_res_btn_playback_shuffle_overlay_png_start;
extern SceUChar8 _binary_res_default_artwork_png_start;
extern SceUChar8 _binary_res_default_artwork_blur_png_start;

static vita2d_texture *Texture_LoadImageBilinear(SceUChar8 *buffer) {
	vita2d_texture *texture = vita2d_load_PNG_buffer(buffer);
	vita2d_texture_set_filters(texture, SCE_GXM_TEXTURE_FILTER_LINEAR, SCE_GXM_TEXTURE_FILTER_LINEAR);
	return texture;
}

void Textures_Load(void) {
	battery_20 = Texture_LoadImageBilinear(&_binary_res_battery_20_png_start);
	battery_30 = Texture_LoadImageBilinear(&_binary_res_battery_30_png_start);
	battery_50 = Texture_LoadImageBilinear(&_binary_res_battery_50_png_start);
	battery_60 = Texture_LoadImageBilinear(&_binary_res_battery_60_png_start);
	battery_80 = Texture_LoadImageBilinear(&_binary_res_battery_80_png_start);
	battery_90 = Texture_LoadImageBilinear(&_binary_res_battery_90_png_start);
	battery_full = Texture_LoadImageBilinear(&_binary_res_battery_full_png_start);

	battery_20_charging = Texture_LoadImageBilinear(&_binary_res_battery_20_charging_png_start);
	battery_30_charging = Texture_LoadImageBilinear(&_binary_res_battery_30_charging_png_start);
	battery_50_charging = Texture_LoadImageBilinear(&_binary_res_battery_50_charging_png_start);
	battery_60_charging = Texture_LoadImageBilinear(&_binary_res_battery_60_charging_png_start);
	battery_80_charging = Texture_LoadImageBilinear(&_binary_res_battery_80_charging_png_start);
	battery_90_charging = Texture_LoadImageBilinear(&_binary_res_battery_90_charging_png_start);
	battery_full_charging = Texture_LoadImageBilinear(&_binary_res_battery_full_charging_png_start);

	battery_low = Texture_LoadImageBilinear(&_binary_res_battery_low_png_start);
	battery_unknown = Texture_LoadImageBilinear(&_binary_res_battery_unknown_png_start);

	icon_audio = Texture_LoadImageBilinear(&_binary_res_icon_audio_png_start);
	icon_file = Texture_LoadImageBilinear(&_binary_res_icon_file_png_start);
	icon_dir = Texture_LoadImageBilinear(&_binary_res_icon_folder_png_start);
	icon_back = Texture_LoadImageBilinear(&_binary_res_icon_back_png_start);

	btn_forward = Texture_LoadImageBilinear(&_binary_res_btn_playback_forward_png_start);
	btn_pause = Texture_LoadImageBilinear(&_binary_res_btn_playback_pause_png_start);
	btn_play = Texture_LoadImageBilinear(&_binary_res_btn_playback_play_png_start);
	btn_repeat = Texture_LoadImageBilinear(&_binary_res_btn_playback_repeat_png_start);
	btn_repeat_overlay = Texture_LoadImageBilinear(&_binary_res_btn_playback_repeat_overlay_png_start);
	btn_rewind = Texture_LoadImageBilinear(&_binary_res_btn_playback_rewind_png_start);
	btn_shuffle = Texture_LoadImageBilinear(&_binary_res_btn_playback_shuffle_png_start);
	btn_shuffle_overlay = Texture_LoadImageBilinear(&_binary_res_btn_playback_shuffle_overlay_png_start);
	default_artwork = Texture_LoadImageBilinear(&_binary_res_default_artwork_png_start);
	default_artwork_blur = Texture_LoadImageBilinear(&_binary_res_default_artwork_blur_png_start);
}

void Textures_Free(void) {
	vita2d_free_texture(default_artwork_blur);
	vita2d_free_texture(default_artwork);
	vita2d_free_texture(btn_shuffle_overlay);
	vita2d_free_texture(btn_shuffle);
	vita2d_free_texture(btn_rewind);
	vita2d_free_texture(btn_repeat_overlay);
	vita2d_free_texture(btn_repeat);
	vita2d_free_texture(btn_play);
	vita2d_free_texture(btn_pause);
	vita2d_free_texture(btn_forward);

	vita2d_free_texture(icon_back);
	vita2d_free_texture(icon_dir);
	vita2d_free_texture(icon_file);
	vita2d_free_texture(icon_audio);

	vita2d_free_texture(battery_unknown);
	vita2d_free_texture(battery_low);

	vita2d_free_texture(battery_full_charging);
	vita2d_free_texture(battery_90_charging);
	vita2d_free_texture(battery_80_charging);
	vita2d_free_texture(battery_60_charging);
	vita2d_free_texture(battery_50_charging);
	vita2d_free_texture(battery_30_charging);
	vita2d_free_texture(battery_20_charging);

	vita2d_free_texture(battery_full);
	vita2d_free_texture(battery_90);
	vita2d_free_texture(battery_80);
	vita2d_free_texture(battery_60);
	vita2d_free_texture(battery_50);
	vita2d_free_texture(battery_30);
	vita2d_free_texture(battery_20);
}
