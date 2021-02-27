#include <kernel.h>
#include <appmgr.h>
#include <shellaudio.h>
#include <stdlib.h>

#include "audio.h"
#include "config.h"
#include "touch_e.h"
#include "utils.h"
#include "menu_audioplayer.h"

extern SceBool playing;
extern SceMusicPlayerServicePlayStatusExtension pb_stats;
unsigned int total_time = 0;

static SceBool seeking = SCE_FALSE;
static unsigned int time_read = 0, seek_frame = 0;

int SHC_Init(char *path) {

	int error = 0;

	sceClibMemset(&pb_stats, 0, sizeof(SceMusicPlayerServicePlayStatusExtension));

	error = sceMusicPlayerServiceInitialize(0);
	if (error < 0)
		return error;

	error = sceMusicPlayerServiceSendEvent(SCE_MUSICCORE_EVENTID_STOP, 0);
	if (error < 0)
		return error;

	error = sceMusicPlayerServiceOpen(path, NULL);
	if (error < 0)
		return error;

	sceMusicPlayerServiceSendEvent(SCE_MUSICCORE_EVENTID_PLAY, 0);

	//Wait until SceShell is ready
	pb_stats.currentState = SCE_MUSICCORE_EVENTID_STOP;
	while (pb_stats.currentState == SCE_MUSICCORE_EVENTID_STOP) {
		sceMusicPlayerServiceGetPlayStatusExtension(&pb_stats);
	}

	SceMusicPlayerServiceTrackInfo data;
	sceMusicPlayerServiceGetTrackInfo(&data);
	total_time = data.duration;

	return 0;
}

SceUInt64 SHC_GetPosition(void) {
	sceMusicPlayerServiceGetPlayStatusExtension(&pb_stats);
	if (!seeking)
		time_read = pb_stats.currentTime;
	else {
		if (seek_frame != pb_stats.currentTime)
			time_read = seek_frame;
		else {
			time_read = pb_stats.currentTime;
			seeking = SCE_FALSE;
		}
	}
	if (pb_stats.currentState == SCE_MUSICCORE_EVENTID_STOP && pb_stats.currentTime == 0)
		playing = SCE_FALSE;
	//This is to prevent audio from stopping when power button is pressed
	if (pb_stats.currentState == SCE_MUSICCORE_EVENTID_STOP && playing && !paused)
		sceMusicPlayerServiceSendEvent(SCE_MUSICCORE_EVENTID_PLAY, 0);

	return time_read;
}

SceUInt64 SHC_GetLength(void) {
	return total_time;
}

SceUInt64 SHC_Seek(SceUInt64 index) {
	seeking = SCE_TRUE;
	seek_frame = (total_time * (index / SEEK_WIDTH_FLOAT));
	sceMusicPlayerServiceSetSeekTime(seek_frame);
	sceMusicPlayerServiceSendEvent(SCE_MUSICCORE_EVENTID_SEEK, 0);

	return -1;
}

void SHC_Term(void) {
	time_read = 0;

	if (metadata.has_meta)
		metadata.has_meta = SCE_FALSE;

	sceMusicPlayerServiceSendEvent(SCE_MUSICCORE_EVENTID_STOP, 0);
	sceMusicPlayerServiceTerminate();
}