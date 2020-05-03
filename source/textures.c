#include <psp2/types.h>

#include "textures.h"

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

extern SceUChar8 _binary_res_toggle_off_png_start;
extern SceUChar8 _binary_res_toggle_on_png_start;
extern SceUChar8 _binary_res_radio_button_checked_png_start;
extern SceUChar8 _binary_res_radio_button_unchecked_png_start;

static vita2d_texture *Texture_LoadImageBilinear(SceUChar8 *buffer) {
	vita2d_texture *texture = vita2d_load_PNG_buffer(buffer);
	vita2d_texture_set_filters(texture, SCE_GXM_TEXTURE_FILTER_LINEAR, SCE_GXM_TEXTURE_FILTER_LINEAR);
	return texture;
}

void Textures_Load(void) {

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

	toggle_on = Texture_LoadImageBilinear(&_binary_res_toggle_on_png_start);
	toggle_off = Texture_LoadImageBilinear(&_binary_res_toggle_off_png_start);
	radio_on = Texture_LoadImageBilinear(&_binary_res_radio_button_checked_png_start);
	radio_off = Texture_LoadImageBilinear(&_binary_res_radio_button_unchecked_png_start);
}

void Textures_Free(void) {
	vita2d_free_texture(radio_off);
	vita2d_free_texture(radio_on);
	vita2d_free_texture(toggle_off);
	vita2d_free_texture(toggle_on);

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
}
