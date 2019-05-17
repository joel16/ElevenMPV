#include <psp2/kernel/threadmgr.h>
#include <stdio.h>
#include <string.h>

#include "audio.h"
#include "vitaaudiolib.h"
#include "fs.h"

#include "flac.h"
#include "mp3.h"
#include "ogg.h"
#include "opus.h"
#include "wav.h"
#include "xm.h"

enum Audio_FileType {
	FILE_TYPE_NONE = 0,
	FILE_TYPE_FLAC = 1,
	FILE_TYPE_MP3 = 2,
	FILE_TYPE_OGG = 3,
	FILE_TYPE_OPUS = 4,
	FILE_TYPE_WAV = 5,
	FILE_TYPE_XM = 6
};

static enum Audio_FileType file_type = FILE_TYPE_NONE;
Audio_Metadata metadata = {0};
static Audio_Metadata empty = {0};
SceBool playing = SCE_TRUE, paused = SCE_FALSE;

static SceUInt32 Audio_GetSampleRate(void) {
	SceUInt32 sample_rate = 0;

	switch(file_type) {
		case FILE_TYPE_FLAC:
			sample_rate = FLAC_GetSampleRate();
			break;

		case FILE_TYPE_MP3:
			sample_rate = MP3_GetSampleRate();
			break;

		case FILE_TYPE_OGG:
			sample_rate = OGG_GetSampleRate();
			break;

		case FILE_TYPE_OPUS:
			sample_rate = OPUS_GetSampleRate();
			break;

		case FILE_TYPE_WAV:
			sample_rate = WAV_GetSampleRate();
			break;

		case FILE_TYPE_XM:
			sample_rate = XM_GetSampleRate();
			break;

		default:
			break;
	}

	return sample_rate;
}

static SceUInt8 Audio_GetChannels(void) {
	SceUInt8 channels = 0;

	switch(file_type) {
		case FILE_TYPE_FLAC:
			channels = FLAC_GetChannels();
			break;

		case FILE_TYPE_MP3:
			channels = MP3_GetChannels();
			break;

		case FILE_TYPE_OGG:
			channels = OGG_GetChannels();
			break;

		case FILE_TYPE_OPUS:
			channels = OPUS_GetChannels();
			break;

		case FILE_TYPE_WAV:
			channels = WAV_GetChannels();
			break;

		case FILE_TYPE_XM:
			channels = XM_GetChannels();
			break;

		default:
			break;
	}

	return channels;
}

static void Audio_Decode(void *buf, unsigned int length, void *userdata) {
	if (playing == SCE_FALSE || paused == SCE_TRUE) {
		short *buf_short = (short *)buf;
		unsigned int count;
		for (count = 0; count < length * 2; count++)
			*(buf_short + count) = 0;
	} 
	else {
		switch(file_type) {
			case FILE_TYPE_FLAC:
				FLAC_Decode(buf, length, userdata);
				break;

			case FILE_TYPE_MP3:
				MP3_Decode(buf, length, userdata);
				break;

			case FILE_TYPE_OGG:
				OGG_Decode(buf, length, userdata);
				break;

			case FILE_TYPE_OPUS:
				OPUS_Decode(buf, length, userdata);
				break;

			case FILE_TYPE_WAV:
				WAV_Decode(buf, length, userdata);
				break;

			case FILE_TYPE_XM:
				XM_Decode(buf, length, userdata);
				break;

			default:
				break;
		}
	}
}

int Audio_Init(const char *path) {
	playing = SCE_TRUE;
	paused = SCE_FALSE;

	if (!strncasecmp(FS_GetFileExt(path), "flac", 4))
		file_type = FILE_TYPE_FLAC;
	else if (!strncasecmp(FS_GetFileExt(path), "mp3", 3))
		file_type = FILE_TYPE_MP3;
	else if (!strncasecmp(FS_GetFileExt(path), "ogg", 3))
		file_type = FILE_TYPE_OGG;
	else if (!strncasecmp(FS_GetFileExt(path), "opus", 4))
		file_type = FILE_TYPE_OPUS;
	else if (!strncasecmp(FS_GetFileExt(path), "wav", 3))
		file_type = FILE_TYPE_WAV;
	else if ((!strncasecmp(FS_GetFileExt(path), "it", 2)) || (!strncasecmp(FS_GetFileExt(path), "mod", 3)) || (!strncasecmp(FS_GetFileExt(path), "s3m", 3)) 
		|| (!strncasecmp(FS_GetFileExt(path), "xm", 2)))
		file_type = FILE_TYPE_XM;

	switch(file_type) {
		case FILE_TYPE_FLAC:
			FLAC_Init(path);
			break;

		case FILE_TYPE_MP3:
			MP3_Init(path);
			break;

		case FILE_TYPE_OGG:
			OGG_Init(path);
			break;

		case FILE_TYPE_OPUS:
			OPUS_Init(path);
			break;

		case FILE_TYPE_WAV:
			WAV_Init(path);
			break;

		case FILE_TYPE_XM:
			XM_Init(path);
			break;

		default:
			break;
	}

	vitaAudioInit(Audio_GetSampleRate(), Audio_GetChannels() == 2? SCE_AUDIO_OUT_PARAM_FORMAT_S16_STEREO : SCE_AUDIO_OUT_PARAM_FORMAT_S16_MONO);
	vitaAudioSetChannelCallback(0, Audio_Decode, NULL);
	return 0;
}

SceBool Audio_IsPaused(void) {
	return paused;
}

void Audio_Pause(void) {
	paused = !paused;
}

void Audio_Stop(void) {
	playing = !playing;
}

SceUInt64 Audio_GetPosition(void) {
	SceUInt64 position = 0;

	switch(file_type) {
		case FILE_TYPE_FLAC:
			position = FLAC_GetPosition();
			break;

		case FILE_TYPE_MP3:
			position = MP3_GetPosition();
			break;

		case FILE_TYPE_OGG:
			position = OGG_GetPosition();
			break;

		case FILE_TYPE_OPUS:
			position = OPUS_GetPosition();
			break;

		case FILE_TYPE_WAV:
			position = WAV_GetPosition();
			break;

		case FILE_TYPE_XM:
			position = XM_GetPosition();
			break;

		default:
			break;
	}

	return position;
}

SceUInt64 Audio_GetLength(void) {
	SceUInt64 length = 0;

	switch(file_type) {
		case FILE_TYPE_FLAC:
			length = FLAC_GetLength();
			break;

		case FILE_TYPE_MP3:
			length = MP3_GetLength();
			break;

		case FILE_TYPE_WAV:
			length = WAV_GetLength();
			break;

		case FILE_TYPE_OGG:
			length = OGG_GetLength();
			break;

		case FILE_TYPE_OPUS:
			length = OPUS_GetLength();
			break;

		case FILE_TYPE_XM:
			length = XM_GetLength();
			break;

		default:
			break;
	}

	return length;
}

SceUInt64 Audio_GetPositionSeconds(void) {
	return (Audio_GetPosition() / Audio_GetSampleRate());
}

SceUInt64 Audio_GetLengthSeconds(void) {
	return (Audio_GetLength() / Audio_GetSampleRate());
}

void Audio_Seek(SceUInt64 index) {
	switch(file_type) {
		case FILE_TYPE_FLAC:
			FLAC_Seek(index);
			break;

		case FILE_TYPE_MP3:
			MP3_Seek(index);
			break;

		case FILE_TYPE_OGG:
			OGG_Seek(index);
			break;

		/*case FILE_TYPE_OPUS:
			OPUS_Seek(index);
			break;*/

		case FILE_TYPE_WAV:
			WAV_Seek(index);
			break;

		case FILE_TYPE_XM:
			XM_Seek(index);
			break;

		default:
			break;
	}
}

void Audio_Term(void) {
	switch(file_type) {
		case FILE_TYPE_FLAC:
			FLAC_Term();
			break;

		case FILE_TYPE_MP3:
			MP3_Term();
			break;

		case FILE_TYPE_OGG:
			OGG_Term();
			break;

		case FILE_TYPE_OPUS:
			OPUS_Term();
			break;

		case FILE_TYPE_WAV:
			WAV_Term();
			break;

		case FILE_TYPE_XM:
			XM_Term();
			break;

		default:
			break;
	}

	playing = SCE_TRUE;
	paused = SCE_FALSE;

	// Clear metadata struct
	metadata = empty;

	vitaAudioSetChannelCallback(0, NULL, NULL); // Clear channel callback
	vitaAudioEndPre();
	sceKernelDelayThread(100 * 1000);
	vitaAudioEnd();
}
