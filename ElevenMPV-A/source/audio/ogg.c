#include <kernel.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include "audio.h"
#include "common.h"
#include "touch_e.h"

static OggVorbis_File ogg;
static FILE* ogg_file;
static vorbis_info *ogg_info = NULL;
static ogg_int64_t samples_read = 0, max_lenth = 0;

static size_t ogg_callback_read(void *ptr, size_t size, size_t count, void *stream) {
	return fread(ptr, size, count, *(FILE **)stream);
}

static int ogg_callback_seek(void *stream, ogg_int64_t offset, int whence) {
	return fseek(*(FILE **)stream, (long) offset, whence);
}

static int ogg_callback_close(void *stream) {
	return fclose(*(FILE **)stream);
}

static long ogg_callback_tell(void *stream) {
	return ftell(*(FILE **)stream);
}

int OGG_Init(const char *path) {
	ogg_file = fopen(path, "r");
	if (ogg_file == NULL)
		return -1;

	ov_callbacks ogg_callbacks;
	ogg_callbacks.read_func = ogg_callback_read;
	ogg_callbacks.seek_func = ogg_callback_seek;
	ogg_callbacks.close_func = ogg_callback_close;
	ogg_callbacks.tell_func = ogg_callback_tell;

	if (R_FAILED(ov_open_callbacks(&ogg_file, &ogg, NULL, 0, ogg_callbacks))) {
		fclose(ogg_file);
		return -1;
	}

	if ((ogg_info = ov_info(&ogg, -1)) == NULL)
		return -1;

	max_lenth = ov_pcm_total(&ogg, -1);

	vorbis_comment *comment = ov_comment(&ogg, -1);
	if (comment != NULL) {

		metadata.has_meta = SCE_TRUE;

		char *value = NULL;

		if ((value = vorbis_comment_query(comment, "title", 0)) != NULL)
			sceClibSnprintf(metadata.title, 260, "%s\n", value);

		if ((value = vorbis_comment_query(comment, "album", 0)) != NULL)
			sceClibSnprintf(metadata.album, 64, "%s\n", value);

		if ((value = vorbis_comment_query(comment, "artist", 0)) != NULL)
			sceClibSnprintf(metadata.artist, 260, "%s\n", value);

		if ((value = vorbis_comment_query(comment, "year", 0)) != NULL)
			sceClibSnprintf(metadata.year, 5, "%s\n", value);

		if ((value = vorbis_comment_query(comment, "comment", 0)) != NULL)
			sceClibSnprintf(metadata.comment, 64, "%s\n", value);

		if ((value = vorbis_comment_query(comment, "genre", 0)) != NULL)
			sceClibSnprintf(metadata.genre, 64, "%s\n", value);
	}

	return 0;
}

SceUInt32 OGG_GetSampleRate(void) {
	return ogg_info->rate;
}

SceUInt8 OGG_GetChannels(void) {
	return ogg_info->channels;
}

static SceUInt64 OGG_FillBuffer(char *out) {
	SceUInt64 samples_read = 0;
	int samples_to_read = (sizeof(SceInt16) * ogg_info->channels) * 960;

	int current_section, samples_just_read;

	while(samples_to_read > 0) {
		samples_just_read = ov_read(&ogg, out, samples_to_read > 960 ? 960 : samples_to_read, 0, 2, 1, &current_section);

		if (samples_just_read < 0)
			return samples_just_read;
		else if (samples_just_read == 0)
			break;

		samples_read += samples_just_read;
		samples_to_read -= samples_just_read;
		out += samples_just_read;
	}

	return samples_read / sizeof(SceInt16);
}

void OGG_Decode(void *buf, unsigned int length, void *userdata) {
	OGG_FillBuffer((char *)buf);
	samples_read = ov_pcm_tell(&ogg);

	if (samples_read >= max_lenth)
		playing = SCE_FALSE;
}

SceUInt64 OGG_GetPosition(void) {
	return samples_read;
}

SceUInt64 OGG_GetLength(void) {
	return max_lenth;
}

SceUInt64 OGG_Seek(SceUInt64 index) {
	ogg_int64_t seek_sample = (max_lenth * (index / SEEK_WIDTH_FLOAT));
	
	if (ov_pcm_seek(&ogg, seek_sample) >= 0) {
		samples_read = seek_sample;
		return samples_read;
	}

	return -1;
}

void OGG_Term(void) {
	samples_read = 0;

	if (metadata.has_meta)
        metadata.has_meta = SCE_FALSE;

	ov_clear(&ogg);
	fclose(ogg_file);
}
