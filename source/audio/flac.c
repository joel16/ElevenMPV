#include "audio.h"
#define DR_FLAC_IMPLEMENTATION
#include "dr_flac.h"

static drflac *flac;
static drflac_uint64 frames_read = 0;

static void FLAC_MetaCallback(void *pUserData, drflac_metadata *pMetadata) {
	if (pMetadata->type == DRFLAC_METADATA_BLOCK_TYPE_PICTURE) {
		if (pMetadata->data.picture.type == DRFLAC_PICTURE_TYPE_COVER_FRONT) {
			metadata.has_meta = SCE_TRUE;

			if ((!strcasecmp(pMetadata->data.picture.mime, "image/jpg")) || (!strcasecmp(pMetadata->data.picture.mime, "image/jpeg")))
				metadata.cover_image = vita2d_load_JPEG_buffer((drflac_uint8 *)pMetadata->data.picture.pPictureData, pMetadata->data.picture.pictureDataSize);
			else if (!strcasecmp(pMetadata->data.picture.mime, "image/png"))
				metadata.cover_image = vita2d_load_PNG_buffer((drflac_uint8 *)pMetadata->data.picture.pPictureData);
		}
	}
}

int FLAC_Init(const char *path) {
	flac = drflac_open_file_with_metadata(path, FLAC_MetaCallback, NULL);
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
	
	if (frames_read == flac->totalPCMFrameCount)
		playing = SCE_FALSE;
}

SceUInt64 FLAC_GetPosition(void) {
	return frames_read;
}

SceUInt64 FLAC_GetLength(void) {
	return flac->totalPCMFrameCount;
}

void FLAC_Term(void) {
	frames_read = 0;

	if (metadata.has_meta)
		metadata.has_meta = SCE_FALSE;

	drflac_close(flac);
}
