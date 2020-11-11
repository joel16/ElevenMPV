#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/clib.h>
#include <psp2/apputil.h>

#include "vitaaudiolib.h"
#include "utils.h"
#include "config.h"

static int audio_ready = 0;
static short vitaAudioSoundBuffer[2][2048][2];
static VITA_audio_channelinfo vitaAudioStatus;
static volatile int audio_terminate = 0;
static int grain = VITA_NUM_AUDIO_SAMPLES;

extern SceUID event_flag_uid;

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
	int vol[2];

	while (audio_terminate == 0) {
		sceKernelWaitEventFlag(event_flag_uid, FLAG_ELEVENMPVA_IS_DECODER_USED, SCE_KERNEL_EVF_WAITMODE_AND, NULL, NULL);
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

		if (audio_ready) {
			sceAppUtilSystemParamGetInt(9, &vol[0]);

			if (config.eq_volume) {
				if (config.eq_mode == 0) {
					vol[1] = vol[0];
					sceAudioOutSetVolume(vitaAudioStatus.handle, SCE_AUDIO_VOLUME_FLAG_L_CH | SCE_AUDIO_VOLUME_FLAG_R_CH, vol);
				}
				else {
					vol[0] = vol[0] / 2;
					vol[1] = vol[0];
					sceAudioOutSetVolume(vitaAudioStatus.handle, SCE_AUDIO_VOLUME_FLAG_L_CH | SCE_AUDIO_VOLUME_FLAG_R_CH, vol);
				}
			}
			else {
				vol[1] = vol[0];
				sceAudioOutSetVolume(vitaAudioStatus.handle, SCE_AUDIO_VOLUME_FLAG_L_CH | SCE_AUDIO_VOLUME_FLAG_R_CH, vol);
			}

			sceAudioOutOutput(vitaAudioStatus.handle, bufptr);
		}
		bufidx = (bufidx ? 0 : 1);
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

	if ((vitaAudioStatus.handle = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_BGM, grain, frequency, mode)) < 0) {
		ret = -1;
		goto error;
	}

	audio_ready = 1;

	vitaAudioStatus.threadhandle = sceKernelCreateThread("ElevenMPVA_audioout", (SceKernelThreadEntry)&vitaAudioChannelThread, 0x100000E0, 0x10000, 0, 0x80000, NULL);

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

void vitaAudioPreSetGrain(int ngrain) {
	grain = ngrain;
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
