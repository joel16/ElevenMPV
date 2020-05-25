#include <psp2/kernel/clib.h>
#include <shellaudio.h>

#include "audio.h"
#include "touch.h"

int sceAppMgrAcquireBgmPortForMusicPlayer(void);

extern SceShellSvcAudioPlaybackStatus pb_stats;

static unsigned int time_read = 0, total_time = 0;

int AAC_Init(char *path) {
	
	int error = 0;

	sceClibMemset(&pb_stats, 0, sizeof(SceShellSvcAudioPlaybackStatus));

	sceAppMgrReleaseBgmPort();

	error = shellAudioInitializeForMusicPlayer(0);
	if (error < 0)
		return error;

	error = shellAudioSendCommandForMusicPlayer(SCE_SHELLAUDIO_STOP, 0);
	if (error < 0)
		return error;

	error = shellAudioSetAudioForMusicPlayer(path, NULL);
	if (error < 0)
		return error;

	shellAudioSendCommandForMusicPlayer(SCE_SHELLAUDIO_PLAY, 0);

	//Wait until SceShell is ready
	pb_stats.currentState = SCE_SHELLAUDIO_STOP;
	while (pb_stats.currentState == SCE_SHELLAUDIO_STOP) {
		shellAudioGetPlaybackStatusForMusicPlayer(&pb_stats);
	}

	SceShellSvcAudioMetadata data;
	shellAudioGetMetadataForMusicPlayer(&data);
	total_time = data.duration;

	return 0;
}

SceUInt64 AAC_GetPosition(void) {
	shellAudioGetPlaybackStatusForMusicPlayer(&pb_stats);
	time_read = pb_stats.currentTime;
	if (pb_stats.currentState == SCE_SHELLAUDIO_STOP && pb_stats.currentTime == 0) {
		playing = SCE_FALSE;
	}
	//This is to prevent audio from stopping when power button is pressed
	if (pb_stats.currentState == SCE_SHELLAUDIO_STOP && playing && !paused)
		shellAudioSendCommandForMusicPlayer(SCE_SHELLAUDIO_PLAY, 0);

	return time_read;
}

SceUInt64 AAC_GetLength(void) {
	return total_time;
}

SceUInt64 AAC_Seek(SceUInt64 index) {
	unsigned int seek_frame = (total_time * (index / SEEK_WIDTH_FLOAT));
	shellAudioSetSeekTimeForMusicPlayer(seek_frame);
	shellAudioSendCommandForMusicPlayer(SCE_SHELLAUDIO_SEEK, 0);

	return -1;
}

void AAC_Term(void) {
	time_read = 0;
	
	shellAudioSendCommandForMusicPlayer(SCE_SHELLAUDIO_STOP, 0);
	shellAudioFinishForMusicPlayer();
	sceAppMgrAcquireBgmPortForMusicPlayer();
}
