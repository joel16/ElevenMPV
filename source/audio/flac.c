#include "audio.h"
#include "config.h"
#define DR_FLAC_IMPLEMENTATION
#include "dr_flac.h"

static drflac *flac;
static drflac_uint64 frames_read = 0;

static void FLAC_ReadTagData(char *source, char *dest) {
	int count = 0, i = 0;
	strcpy(dest, "");

	for (i = 0; i < strlen(source); i++) {
		if ((unsigned char)source[i] >= 0x20 && (unsigned char)source[i] <= 0xfd) {
			dest[count] = source[i];
			if (++count >= 256)
				break;
		}
	}

	dest[count] = '\0';
}

static void FLAC_SplitVorbisComments(char *comment, char *name, char *value){
	char *result = NULL;
	result = strtok(comment, "=");
	int count = 0;

	while(result != NULL && count < 2) {
		if (strlen(result) > 0) {
			switch (count){
				case 0:
					strncpy(name, result, 30);
					name[30] = '\0';
					break;

				case 1:
					FLAC_ReadTagData(result, value);
					value[256] = '\0';
					break;
			}

			count++;
		}

		result = strtok(NULL, "=");
	}
}

static void FLAC_MetaCallback(void *pUserData, drflac_metadata *pMetadata) {
	char tag[32];
	char value[31];

	if (pMetadata->type == DRFLAC_METADATA_BLOCK_TYPE_VORBIS_COMMENT) {
		drflac_vorbis_comment_iterator iterator;
		drflac_uint32 comment_length;
		const char *const_comment_str;
		char *comment_str;

		drflac_init_vorbis_comment_iterator(&iterator, pMetadata->data.vorbis_comment.commentCount, pMetadata->data.vorbis_comment.pComments);

		while((const_comment_str = drflac_next_vorbis_comment(&iterator, &comment_length)) != NULL) {
			comment_str = strdup(const_comment_str);
			FLAC_SplitVorbisComments(comment_str, tag, value);
			
			if (!strcasecmp(tag, "title"))
				snprintf(metadata.title, 32, "%s\n", value);
			if (!strcasecmp(tag, "album"))
				snprintf(metadata.album, 32, "%s\n", value);
			if (!strcasecmp(tag, "artist"))
				snprintf(metadata.artist, 32, "%s\n", value);
			if (!strcasecmp(tag, "year"))
				snprintf(metadata.year, 32, "%s\n", value);
			if (!strcasecmp(tag, "comment"))
				snprintf(metadata.comment, 32, "%s\n", value);
			if (!strcasecmp(tag, "genre"))
				snprintf(metadata.genre, 32, "%s\n", value);
		}
	}

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
	flac = config.meta_flac? drflac_open_file_with_metadata(path, FLAC_MetaCallback, NULL) : drflac_open_file(path);
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
