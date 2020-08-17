#include <psp2/kernel/clib.h>
#include <psp2/libc.h>
#include <psp2/appmgr.h>
#include <psp2/kernel/iofilemgr.h>
#include <shellaudio.h>

#include "audio.h"
#include "config.h"
#include "touch.h"
#include "id3.h"
#include "utils.h"
#include "menu_audioplayer.h"

int sceAppMgrAcquireBgmPortForMusicPlayer(void);

extern SceBool playing;
extern SceShellSvcAudioPlaybackStatus pb_stats;
extern SceBool ext_cover_loaded;

static unsigned int time_read = 0, total_time = 0;

int MP3_Init(char *path) {

	sceClibMemset(&pb_stats, 0, sizeof(SceShellSvcAudioPlaybackStatus));
	sceAppMgrReleaseBgmPort();

	int error = shellAudioInitializeForMusicPlayer(0);
	if (error < 0)
		return error;

	error = shellAudioSendCommandForMusicPlayer(SCE_SHELLAUDIO_STOP, 0);
	if (error < 0)
		return error;

	error = shellAudioSetAudioForMusicPlayer(path, NULL);
	if (error < 0)
		return error;

	ID3Tag *ID3 = sceLibcMalloc(sizeof(ID3Tag));
	ParseID3(path, ID3);

	char* meta_ptr = (char *)ID3;

	for (int i = 0; i < 792; i++) {
		if (meta_ptr[i] != '\0') {
			metadata.has_meta = SCE_TRUE;
			break;
		}
	}

	if (metadata.has_meta) {
		sceClibStrncpy(metadata.title, ID3->ID3Title, 260);
		sceClibStrncpy(metadata.artist, ID3->ID3Artist, 260);
		sceClibStrncpy(metadata.album, ID3->ID3Album, 64);
		sceClibStrncpy(metadata.year, ID3->ID3Year, 5);
		sceClibStrncpy(metadata.genre, ID3->ID3GenreText, 64);
	}

	//Wait until SceShell is ready, but use that time to load cover
	shellAudioSendCommandForMusicPlayer(SCE_SHELLAUDIO_PLAY, 0);
	pb_stats.currentState = SCE_SHELLAUDIO_STOP;

	if (ID3->ID3EncapsulatedPictureType == JPEG_IMAGE) {
		Menu_UnloadExternalCover();
		SceUID fd = sceIoOpen(path, SCE_O_RDONLY, 0);
		if (fd >= 0) {
			char *buffer = sceLibcMalloc(ID3->ID3EncapsulatedPictureLength);
			if (buffer) {
				sceIoLseek32(fd, ID3->ID3EncapsulatedPictureOffset, SCE_SEEK_SET);
				sceIoRead(fd, buffer, ID3->ID3EncapsulatedPictureLength);
				sceIoClose(fd);

				vita2d_JPEG_ARM_decoder_initialize();
				metadata.cover_image = vita2d_load_JPEG_ARM_buffer(buffer, ID3->ID3EncapsulatedPictureLength, 0, 0, 0);
				vita2d_JPEG_ARM_decoder_finish();

				sceLibcFree(buffer);
			}
		}
	}
	else if (ID3->ID3EncapsulatedPictureType == PNG_IMAGE) {
		Menu_UnloadExternalCover();
		SceUID fd = sceIoOpen(path, SCE_O_RDONLY, 0);
		if (fd >= 0) {
			char *buffer = sceLibcMalloc(ID3->ID3EncapsulatedPictureLength);
			if (buffer) {
				sceIoLseek32(fd, ID3->ID3EncapsulatedPictureOffset, SCE_SEEK_SET);
				sceIoRead(fd, buffer, ID3->ID3EncapsulatedPictureLength);
				sceIoClose(fd);

				metadata.cover_image = vita2d_load_PNG_buffer(buffer);

				sceLibcFree(buffer);
			}
		}
	}

	sceLibcFree(ID3);

	//Wait until SceShell is ready
	while (pb_stats.currentState == SCE_SHELLAUDIO_STOP) {
		shellAudioGetPlaybackStatusForMusicPlayer(&pb_stats);
	}

	SceShellSvcAudioMetadata data;
	shellAudioGetMetadataForMusicPlayer(&data);
	total_time = data.duration;

	return 0;
}

SceUInt64 MP3_GetPosition(void) {
	shellAudioGetPlaybackStatusForMusicPlayer(&pb_stats);
	time_read = pb_stats.currentTime;
	if (pb_stats.currentState == SCE_SHELLAUDIO_STOP && pb_stats.currentTime == 0)
		playing = SCE_FALSE;
	//This is to prevent audio from stopping when power button is pressed
	if (pb_stats.currentState == SCE_SHELLAUDIO_STOP && playing && !paused)
		shellAudioSendCommandForMusicPlayer(SCE_SHELLAUDIO_PLAY, 0);

	return time_read;
}

SceUInt64 MP3_GetLength(void) {
	return total_time;
}

SceUInt64 MP3_Seek(SceUInt64 index) {
	unsigned int seek_frame = (total_time * (index / SEEK_WIDTH_FLOAT));
	shellAudioSetSeekTimeForMusicPlayer(seek_frame);
	shellAudioSendCommandForMusicPlayer(SCE_SHELLAUDIO_SEEK, 0);

	return -1;
}

void MP3_Term(void) {
	time_read = 0;

	if (metadata.has_meta)
		metadata.has_meta = SCE_FALSE;

	shellAudioSendCommandForMusicPlayer(SCE_SHELLAUDIO_STOP, 0);
	shellAudioFinishForMusicPlayer();
	sceAppMgrAcquireBgmPortForMusicPlayer();
}
