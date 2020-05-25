#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/clib.h>

#include "vitaaudiolib.h"

static int audio_ready = 0;
static short vitaAudioSoundBuffer[2][VITA_NUM_AUDIO_SAMPLES][2];
static VITA_audio_channelinfo vitaAudioStatus;
static volatile int audio_terminate = 0;

extern SceBool isSceShellUsed;

void vitaAudioSetVolume(int left, int right) {

	int vol[2] = { left, right };

	if (vol[0] > SCE_AUDIO_OUT_MAX_VOL)
		vol[0] = SCE_AUDIO_OUT_MAX_VOL;

	if (vol[1] > SCE_AUDIO_OUT_MAX_VOL)
		vol[1] = SCE_AUDIO_OUT_MAX_VOL;

	sceAudioOutSetVolume(vitaAudioStatus.handle, SCE_AUDIO_VOLUME_FLAG_L_CH | SCE_AUDIO_VOLUME_FLAG_R_CH, vol);
}

void vitaAudioSetChannelCallback(vitaAudioCallback_t callback, void *userdata) {
	volatile VITA_audio_channelinfo *pci = &vitaAudioStatus;

	if (callback == 0)
		pci->callback = 0;
	else
		pci->callback = callback;
}

static int vitaAudioChannelThread(unsigned int args, void *argp) {
	volatile int bufidx = 0;
	void *bufptr;

	while (audio_terminate == 0) {
		if (!isSceShellUsed) {
			bufptr = &vitaAudioSoundBuffer[bufidx];

			if (vitaAudioStatus.callback) {
				vitaAudioStatus.callback(bufptr, VITA_NUM_AUDIO_SAMPLES, vitaAudioStatus.userdata);
			}
			else {
				unsigned int *ptr = bufptr;
				int i;
				for (i = 0; i < VITA_NUM_AUDIO_SAMPLES; ++i)
					*(ptr++) = 0;
			}

			if (audio_ready)
				sceAudioOutOutput(vitaAudioStatus.handle, bufptr);
			bufidx = (bufidx ? 0 : 1);
		}
		else
			sceKernelDelayThread(100000);
	}

	sceKernelExitThread(0);
	return(0);
}

int vitaAudioInit(int frequency, SceAudioOutMode mode) {
	int ret = 0;

	audio_terminate = 0;
	audio_ready = 0;

	vitaAudioStatus.handle = -1;
	vitaAudioStatus.threadhandle = -1;
	vitaAudioStatus.callback = 0;
	vitaAudioStatus.userdata = 0;

	if ((vitaAudioStatus.handle = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_BGM, VITA_NUM_AUDIO_SAMPLES, frequency, mode)) < 0) {
		ret = -1;
		goto error;
	}

	audio_ready = 1;

	vitaAudioStatus.threadhandle = sceKernelCreateThread("ElevenMPVA_audioout", (SceKernelThreadEntry)&vitaAudioChannelThread, 64, 0x1000, 0, 0x80000, NULL);

	if (vitaAudioStatus.threadhandle < 0) {
		vitaAudioStatus.threadhandle = -1;
		ret = -1;
		goto error;
	}

	ret = sceKernelStartThread(vitaAudioStatus.threadhandle, 0, NULL);

error:

	if (ret < 0) {
		audio_terminate = 1;
		if (vitaAudioStatus.threadhandle != -1)
			sceKernelDeleteThread(vitaAudioStatus.threadhandle);

		vitaAudioStatus.threadhandle = -1;

		audio_ready = 0;
		return 0;
	}

	return 1;
}

void vitaAudioEndPre(void) {
	audio_ready = 0;
	audio_terminate = 1;
}

void vitaAudioEnd(void) {
	audio_ready = 0;
	audio_terminate = 1;

	if (vitaAudioStatus.threadhandle != -1)
		sceKernelDeleteThread(vitaAudioStatus.threadhandle);

	vitaAudioStatus.threadhandle = -1;

	if (vitaAudioStatus.handle != -1) {
		sceAudioOutReleasePort(vitaAudioStatus.handle);
		vitaAudioStatus.handle = -1;
		}
}
