#include <psp2/kernel/clib.h>
#include <psp2/io/fcntl.h> 
#include <psp2/libc.h>

#include "audio.h"
#include "config.h"
#include "touch.h"
#include "dr_flac.h"
#include "menu_audioplayer.h"

static drflac *flac;
static drflac_uint64 frames_read = 0;
static SceUID flac_fd = 0;

void metadata_cb(void* pUserData, drflac_metadata* pMetadata) {

	if (pMetadata->type == DRFLAC_METADATA_BLOCK_TYPE_PICTURE) {
		if (pMetadata->data.picture.type == DRFLAC_PICTURE_TYPE_COVER_FRONT) {
			if (!sceClibStrncasecmp("image/jpg", pMetadata->data.picture.mime, 9) || !sceClibStrncasecmp("image/jpeg", pMetadata->data.picture.mime, 10)) {
				Menu_UnloadExternalCover();
				vita2d_JPEG_ARM_decoder_initialize();
				metadata.cover_image = vita2d_load_JPEG_ARM_buffer(pMetadata->data.picture.pPictureData, pMetadata->data.picture.pictureDataSize, 0, 0, 0);
				vita2d_JPEG_ARM_decoder_finish();
				metadata.has_meta = SCE_TRUE;
			}
			else if (!sceClibStrncasecmp("image/png", pMetadata->data.picture.mime, 9)) {
				Menu_UnloadExternalCover();
				metadata.cover_image = vita2d_load_PNG_buffer(pMetadata->data.picture.pPictureData);
				metadata.has_meta = SCE_TRUE;
			}
		}
	}
	else if (pMetadata->type == DRFLAC_METADATA_BLOCK_TYPE_VORBIS_COMMENT) {
		unsigned int len;
		drflac_vorbis_comment_iterator iter;
		drflac_init_vorbis_comment_iterator(&iter, pMetadata->data.vorbis_comment.commentCount, pMetadata->data.vorbis_comment.pComments);
		for (int i = 0; i < pMetadata->data.vorbis_comment.commentCount; i++) {
			const char* tag = drflac_next_vorbis_comment(&iter, &len);

			if (!sceClibStrncasecmp("TITLE=", tag, 6)) {
				metadata.has_meta = SCE_TRUE;
				sceClibSnprintf(metadata.title, len, "%s\n", tag + 6);
			}

			if (!sceClibStrncasecmp("ALBUM=", tag, 6)) {
				metadata.has_meta = SCE_TRUE;
				sceClibSnprintf(metadata.album, len, "%s\n", tag + 6);
			}

			if (!sceClibStrncasecmp("ARTIST=", tag, 7)) {
				metadata.has_meta = SCE_TRUE;
				sceClibSnprintf(metadata.artist, len, "%s\n", tag + 7);
			}

			if (!sceClibStrncasecmp("DATE=", tag, 5)) {
				metadata.has_meta = SCE_TRUE;
				sceClibSnprintf(metadata.year, 4, "%d\n", atoi(tag + 5));
			}

			if (!sceClibStrncasecmp("COMMENT=", tag, 8)) {
				metadata.has_meta = SCE_TRUE;
				sceClibSnprintf(metadata.comment, len, "%s\n", tag + 8);
			}

			if (!sceClibStrncasecmp("GENRE=", tag, 6)) {
				metadata.has_meta = SCE_TRUE;
				sceClibSnprintf(metadata.genre, len, "%s\n", tag + 6);
			}
		}
	}
}

size_t dr_flac_read(void* pUserData, void* pBufferOut, size_t bytesToRead) {
	return sceIoRead(flac_fd, pBufferOut, bytesToRead);
}

drflac_bool32 dr_flac_seek(void* pUserData, int offset, drflac_seek_origin origin) {
	if (sceIoLseek32(flac_fd, offset, origin) < 0)
		return DRFLAC_FALSE;
	else
		return DRFLAC_TRUE;
}

int FLAC_Init(const char *path) {

	flac_fd = sceIoOpen(path, SCE_O_RDONLY, 0777);

	flac = drflac_open_with_metadata(dr_flac_read, dr_flac_seek, metadata_cb, NULL, NULL);
	if (flac == NULL)
		return -1;

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

	sceIoClose(flac_fd);
	drflac_close(flac);
}
