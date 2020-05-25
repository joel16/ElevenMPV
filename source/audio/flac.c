#include <psp2/kernel/clib.h>
#include <ctype.h>
#include <FLAC/metadata.h>

#include "audio.h"
#include "config.h"
#include "touch.h"
#define DR_FLAC_IMPLEMENTATION
#define DRFLAC_ARM
#define DRFLAC_SUPPORT_NEON
#define DR_FLAC_NO_OGG
#include "dr_flac.h"

static drflac *flac;
static drflac_uint64 frames_read = 0;

int FLAC_Init(const char *path) {
	flac = drflac_open_file(path, NULL);
	if (flac == NULL)
		return -1;

	FLAC__StreamMetadata *tags;
	if (FLAC__metadata_get_tags(path, &tags)) {
		for (int i = 0; i < tags->data.vorbis_comment.num_comments; i++)  {
			char *tag = (char *)tags->data.vorbis_comment.comments[i].entry;

			if (!sceClibStrncasecmp("TITLE=", tag, 6)) {
				metadata.has_meta = SCE_TRUE;
				sceClibSnprintf(metadata.title, 31, "%s\n", tag + 6);
			}

			if (!sceClibStrncasecmp("ALBUM=", tag, 6)) {
				metadata.has_meta = SCE_TRUE;
				sceClibSnprintf(metadata.album, 31, "%s\n", tag + 6);
			}

			if (!sceClibStrncasecmp("ARTIST=", tag, 7)) {
				metadata.has_meta = SCE_TRUE;
				sceClibSnprintf(metadata.artist, 31, "%s\n", tag + 7);
			}

			if (!sceClibStrncasecmp("DATE=", tag, 5)) {
				metadata.has_meta = SCE_TRUE;
				sceClibSnprintf(metadata.year, 31, "%d\n", atoi(tag + 5));
			}

			if (!sceClibStrncasecmp("COMMENT=", tag, 8)) {
				metadata.has_meta = SCE_TRUE;
				sceClibSnprintf(metadata.comment, 31, "%s\n", tag + 8);
			}

			if (!sceClibStrncasecmp("GENRE=", tag, 6)) {
				metadata.has_meta = SCE_TRUE;
				sceClibSnprintf(metadata.genre, 31, "%s\n", tag + 6);
			}
		}
	}

	if (tags)
		FLAC__metadata_object_delete(tags);

	return 0;
}

SceUInt32 FLAC_GetSampleRate(void) {
	return flac->sampleRate;
}

SceUInt8 FLAC_GetChannels(void) {
	return flac->channels;
}

void FLAC_Decode(void *buf, unsigned int length, void *userdata) {
	frames_read += drflac_read_pcm_frames_s16(flac, (drflac_uint64)length, (drflac_int16 *)buf);
	
	if (frames_read >= flac->totalPCMFrameCount)
		playing = SCE_FALSE;
}

SceUInt64 FLAC_GetPosition(void) {
	return frames_read;
}

SceUInt64 FLAC_GetLength(void) {
	return flac->totalPCMFrameCount;
}

SceUInt64 FLAC_Seek(SceUInt64 index) {
	drflac_uint64 seek_frame = (flac->totalPCMFrameCount * (index / SEEK_WIDTH_FLOAT));
	
	if (drflac_seek_to_pcm_frame(flac, seek_frame) == DRFLAC_TRUE) {
		frames_read = seek_frame;
		return frames_read;
	}

	return -1;
}

void FLAC_Term(void) {
	frames_read = 0;

	if (metadata.has_meta)
		metadata.has_meta = SCE_FALSE;

	drflac_close(flac);
}

// Functions needed for libFLAC

int chmod(const char *pathname, mode_t mode) {
	return 0;
}

int chown(const char *path, int owner, int group) {
	return 0;
}

int utime(const char *filename, const void *buf) {
	return 0;
}
