#include <string.h>

#include "audio.h"
#include "config.h"
#include "opusfile.h"

static OggOpusFile *opus;
static ogg_int64_t samples_read = 0, max_samples = 0;

int OPUS_Init(const char *path) {
	int error = 0;

	if ((opus = op_open_file(path, &error)) == NULL)
		return OP_FALSE;

	if ((error = op_current_link(opus)) < 0)
		return OP_FALSE;

	max_samples = op_pcm_total(opus, -1);

	const OpusTags *tags = op_tags(opus, 0);

	if (opus_tags_query_count(tags, "title") > 0) {
		metadata.has_meta = SCE_TRUE;
		snprintf(metadata.title, 31, "%s\n", opus_tags_query(tags, "title", 0));
	}

	if (opus_tags_query_count(tags, "album") > 0) {
		metadata.has_meta = SCE_TRUE;
		snprintf(metadata.album, 31, "%s\n", opus_tags_query(tags, "album", 0));
	}

	if (opus_tags_query_count(tags, "artist") > 0) {
		metadata.has_meta = SCE_TRUE;
		snprintf(metadata.artist, 31, "%s\n", opus_tags_query(tags, "artist", 0));
	}

	if (opus_tags_query_count(tags, "date") > 0) {
		metadata.has_meta = SCE_TRUE;
		snprintf(metadata.year, 11, "%s\n", opus_tags_query(tags, "date", 0));
	}

	if (opus_tags_query_count(tags, "comment") > 0) {
		metadata.has_meta = SCE_TRUE;
		snprintf(metadata.comment, 31, "%s\n", opus_tags_query(tags, "comment", 0));
	}

	if (opus_tags_query_count(tags, "genre") > 0) {
		metadata.has_meta = SCE_TRUE;
		snprintf(metadata.genre, 31, "%s\n", opus_tags_query(tags, "genre", 0));
	}

	if ((opus_tags_query_count(tags, "METADATA_BLOCK_PICTURE") > 0) && (config.meta_opus)) {
		metadata.has_meta = SCE_TRUE;

		OpusPictureTag picture_tag = { 0 };
		opus_picture_tag_init(&picture_tag);
		const char* metadata_block = opus_tags_query(tags, "METADATA_BLOCK_PICTURE", 0);

		int error = opus_picture_tag_parse(&picture_tag, metadata_block);
		if (error == 0) {
			if (picture_tag.type == 3) {
				if (picture_tag.format == OP_PIC_FORMAT_JPEG)
					metadata.cover_image = vita2d_load_JPEG_buffer(picture_tag.data, picture_tag.data_length);
				else if (picture_tag.format == OP_PIC_FORMAT_PNG)
					metadata.cover_image = vita2d_load_PNG_buffer(picture_tag.data);
			}
		}

		opus_picture_tag_clear(&picture_tag);
	}

	return 0;
}

SceUInt32 OPUS_GetSampleRate(void) {
	return 48000;
}

SceUInt8 OPUS_GetChannels(void) {
	return 2;
}

void OPUS_Decode(void *buf, unsigned int length, void *userdata) {
	int read = op_read_stereo(opus, (opus_int16 *)buf, (int)length * (sizeof(SceInt16) * 2));
	if (read)
		samples_read = op_pcm_tell(opus);

	if (samples_read >= max_samples)
		playing = SCE_FALSE;
}

SceUInt64 OPUS_GetPosition(void) {
	return samples_read;
}

SceUInt64 OPUS_GetLength(void) {
	return max_samples;
}

SceUInt64 OPUS_Seek(SceUInt64 index) {
	if (op_seekable(opus) >= 0) {
		ogg_int64_t seek_sample = (max_samples * (index / 450.0));
	
		if (op_pcm_seek(opus, seek_sample) >= 0) {
			samples_read = seek_sample;
			return samples_read;
		}
	}

	return -1;
}

void OPUS_Term(void) {
	samples_read = 0;

	if (metadata.has_meta)
        metadata.has_meta = SCE_FALSE;

	op_free(opus);
}
