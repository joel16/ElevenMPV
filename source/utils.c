#include <psp2/apputil.h>
#include <psp2/io/dirent.h>
#include <psp2/system_param.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/notificationutil.h>
#include <psp2/kernel/clib.h>
#include <psp2/sysmodule.h> 
#include <psp2/shellutil.h>
#include <psp2/display.h>
#include <psp2/power.h> 
#include <psp2/ime.h> 
#include <psp2/appmgr.h> 
#include <psp2/libc.h>
#include <shellaudio.h> 
#include <taihen.h>

#include "utils.h"
#include "config.h"
#include "touch.h"
#include "audio.h"
#include "common.h"
#include "vitaaudiolib.h"

#define ROUND_UP(x, a)	((((unsigned int)x)+((a)-1u))&(~((a)-1u)))

typedef struct SceAppMgrEvent { // size is 0x64
	int event;				/* Event ID */
	SceUID appId;			/* Application ID. Added when required by the event */
	char  param[56];		/* Parameters to pass with the event */
} SceAppMgrEvent;

int _sceAppMgrReceiveEvent(SceAppMgrEvent *appEvent);
int sceAppMgrQuitForNonSuspendableApp(void);
int sceAppMgrAcquireBgmPortForMusicPlayer(void);

extern SceUID main_thread_uid;
extern SceUID event_flag_uid;

#ifdef DEBUG
extern SceAppMgrBudgetInfo budget_info;
#endif // DEBUG

static SceCtrlData pad, old_pad;
static SceBool ime_module_loaded = SCE_FALSE;
static SceUInt32 libime_work[SCE_IME_WORK_BUFFER_SIZE / sizeof(SceUInt32)];

static SceNotificationUtilProgressInitParam notify_init_param;
static SceNotificationUtilProgressUpdateParam notify_update_param;

void Utils_SetMax(int *set, int value, int max) {
	if (*set > max)
		*set = value;
}

void Utils_SetMin(int *set, int value, int min) {
	if (*set < min)
		*set = value;
}

int Utils_ReadControls(SceSize argc, void* argv) {

	while (SCE_TRUE) {
		sceKernelWaitEventFlag(event_flag_uid, FLAG_ELEVENMPVA_IS_FG, SCE_KERNEL_EVF_WAITMODE_AND, NULL, NULL);

		sceClibMemset(&pad, 0, sizeof(SceCtrlData));
		sceCtrlPeekBufferPositive(0, &pad, 1);

		pressed = pad.buttons & ~old_pad.buttons;

		sceClibMemcpy(&old_pad, &pad, sizeof(SceCtrlData));

		//for controls update need to check every frame at 60fps
		sceDisplayWaitVblankStart();
	}
	return 0;
}

void Utils_Exit(void)
{
	sceNotificationUtilCleanHistory();
	SceUID	modid = taiLoadStartKernelModule("ux0:app/ELEVENMPV/module/kernel/exit_module.skprx", 0, NULL, 0);
	sceAppMgrQuitForNonSuspendableApp();
	taiStopUnloadKernelModule(modid, 0, NULL, 0, NULL, NULL);
}

void Utils_NotificationProgressBegin(SceNotificationUtilProgressInitParam* init_param) {
	sceClibMemcpy(&notify_init_param, init_param, sizeof(SceNotificationUtilProgressInitParam));
	sceKernelSetEventFlag(event_flag_uid, FLAG_ELEVENMPVA_IS_READY_NOTIFY);
}

void Utils_NotificationProgressUpdate(SceNotificationUtilProgressUpdateParam* update_param) {
	sceClibMemcpy(&notify_update_param, update_param, sizeof(SceNotificationUtilProgressUpdateParam));
	sceKernelSetEventFlag(event_flag_uid, FLAG_ELEVENMPVA_IS_UPDATE_NOTIFY);
}

void Utils_NotificationEnd(void) {
	sceKernelSetEventFlag(event_flag_uid, FLAG_ELEVENMPVA_IS_END_NOTIFY);
}

void Utils_NotificationEventHandler(int a1) {
	switch (config.notify_mode) {
	case 1:
		sceKernelSetEventFlag(event_flag_uid, FLAG_ELEVENMPVA_IS_FINISHED_PLAYLIST);
		break;
	case 2:
		Config_Save(config);
		if (!Utils_IsDecoderUsed()) {
			shellAudioSendCommandForMusicPlayer(SCE_SHELLAUDIO_STOP, 0);
			shellAudioFinishForMusicPlayer();
		}
		Utils_Exit();
		break;
	}
}

int Utils_AppStatusWatchdog(SceSize argc, void* argv) {
	int vol;
	SceAppMgrEvent appEvent;
	while (SCE_TRUE) {

		//check app status

		_sceAppMgrReceiveEvent(&appEvent);
		switch (appEvent.event) {
		case SCE_APP_EVENT_ON_ACTIVATE:
			if (paused && Utils_IsDecoderUsed())
				sceAppMgrAcquireBgmPortForMusicPlayer();
			sceKernelSetEventFlag(event_flag_uid, FLAG_ELEVENMPVA_IS_FG);
			sceKernelChangeThreadPriority(main_thread_uid, 160);
			break;
		case SCE_APP_EVENT_ON_DEACTIVATE:
			if (paused)
				sceAppMgrReleaseBgmPort();
			sceKernelClearEventFlag(event_flag_uid, ~FLAG_ELEVENMPVA_IS_FG);
			sceKernelChangeThreadPriority(main_thread_uid, 191);
			break;
		case SCE_APP_EVENT_REQUEST_QUIT:
			Config_Save(config);
			if (!Utils_IsDecoderUsed()) {
				shellAudioSendCommandForMusicPlayer(SCE_SHELLAUDIO_STOP, 0);
				shellAudioFinishForMusicPlayer();
			}
			Utils_Exit();
			break;
		}

		//check volume

		sceAppUtilSystemParamGetInt(9, &vol);

		if (config.eq_volume) {
			if (config.eq_mode == 0)
				vitaAudioSetVolume(vol, vol);
			else
				vitaAudioSetVolume(vol / 2, vol / 2);
		}

		//check notifications

		if (config.notify_mode > 0) {
			if (!sceKernelPollEventFlag(event_flag_uid, FLAG_ELEVENMPVA_IS_READY_NOTIFY, SCE_KERNEL_EVF_WAITMODE_AND, NULL)) {
				sceNotificationUtilProgressBegin(&notify_init_param);
				sceKernelClearEventFlag(event_flag_uid, ~FLAG_ELEVENMPVA_IS_READY_NOTIFY);
			}
			else if (!sceKernelPollEventFlag(event_flag_uid, FLAG_ELEVENMPVA_IS_END_NOTIFY, SCE_KERNEL_EVF_WAITMODE_AND, NULL)) {
				SceNotificationUtilProgressFinishParam finish_param;
				sceClibMemset(&finish_param, 0, sizeof(SceNotificationUtilProgressFinishParam));
				Utils_Utf8ToUtf16(finish_param.notificationText, "Playback has been stopped.");
				sceNotificationUtilProgressFinish(&finish_param);
				sceKernelClearEventFlag(event_flag_uid, ~FLAG_ELEVENMPVA_IS_END_NOTIFY);
			}
			else if (!sceKernelPollEventFlag(event_flag_uid, FLAG_ELEVENMPVA_IS_UPDATE_NOTIFY, SCE_KERNEL_EVF_WAITMODE_AND, NULL)) {
				sceNotificationUtilProgressUpdate(&notify_update_param);
				sceKernelClearEventFlag(event_flag_uid, ~FLAG_ELEVENMPVA_IS_UPDATE_NOTIFY);
			}
		}

#ifdef DEBUG
		//malloc_stats();
		sceAppMgrGetBudgetInfo(&budget_info);
		/*sceClibPrintf("----- EMPA-A BUDGET -----");
		sceClibPrintf("LPDDR2: %f MB\n", budget_info.freeLPDDR2 / 1024.0 / 1024.0);*/
#endif

		sceKernelDelayThread(100 * 1000);
	}

	return 0;
}

int Utils_InitAppUtil(void) {

	/* sceCtrl rules */

	SceCtrlRapidFireRule scroll_rule_up;
	sceClibMemset(&scroll_rule_up, 0, sizeof(SceCtrlRapidFireRule));

	scroll_rule_up.Mask = SCE_CTRL_UP;
	scroll_rule_up.Trigger = SCE_CTRL_UP;
	scroll_rule_up.Target = SCE_CTRL_UP;
	scroll_rule_up.Delay = 20;
	scroll_rule_up.Make = 2;
	scroll_rule_up.Break = 2;

	sceCtrlSetRapidFire(0, 0, &scroll_rule_up);

	SceCtrlRapidFireRule scroll_rule_down;
	sceClibMemset(&scroll_rule_down, 0, sizeof(SceCtrlRapidFireRule));

	scroll_rule_down.Mask = SCE_CTRL_DOWN;
	scroll_rule_down.Trigger = SCE_CTRL_DOWN;
	scroll_rule_down.Target = SCE_CTRL_DOWN;
	scroll_rule_down.Delay = 20;
	scroll_rule_down.Make = 2;
	scroll_rule_down.Break = 2;

	sceCtrlSetRapidFire(0, 1, &scroll_rule_down);

	sceSysmoduleLoadModule(SCE_SYSMODULE_NOTIFICATION_UTIL);

	sceClibMemset(&notify_init_param, 0, sizeof(SceNotificationUtilProgressInitParam));
	sceClibMemset(&notify_update_param, 0, sizeof(SceNotificationUtilProgressUpdateParam));

	SceUID status_watchdog = sceKernelCreateThread("app_status_watchdog", Utils_AppStatusWatchdog, 191, 0x1000, 0, 0, NULL);
	sceKernelStartThread(status_watchdog, 0, NULL);

	SceUID ctrl_watchdog = sceKernelCreateThread("controls_watchdog", Utils_ReadControls, 160, 0x1000, 0, 0, NULL);
	sceKernelStartThread(ctrl_watchdog, 0, NULL);

	SceAppUtilInitParam init;
	SceAppUtilBootParam boot;
	sceClibMemset(&init, 0, sizeof(SceAppUtilInitParam));
	sceClibMemset(&boot, 0, sizeof(SceAppUtilBootParam));
	
	int ret = 0;
	
	if (R_FAILED(ret = sceAppUtilInit(&init, &boot)))
		return ret;

	if (R_FAILED(ret = sceAppUtilMusicMount()))
		return ret;
	
	return 0;
}

int Utils_GetEnterButton(void) {
	int button = 0;
	sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_ENTER_BUTTON, &button);
	
	if (button == SCE_SYSTEM_PARAM_ENTER_BUTTON_CIRCLE)
		return SCE_CTRL_CIRCLE;
	else
		return SCE_CTRL_CROSS;

	return 0;
}

int Utils_GetCancelButton(void) {
	int button = 0;
	sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_ENTER_BUTTON, &button);
	
	if (button == SCE_SYSTEM_PARAM_ENTER_BUTTON_CIRCLE)
		return SCE_CTRL_CROSS;
	else
		return SCE_CTRL_CIRCLE;

	return 0;
}

int Utils_Alphasort(const void *p1, const void *p2) {
	SceIoDirent *entryA = (SceIoDirent *) p1;
	SceIoDirent *entryB = (SceIoDirent *) p2;
	
	if ((SCE_S_ISDIR(entryA->d_stat.st_mode)) && !(SCE_S_ISDIR(entryB->d_stat.st_mode)))
		return -1;
	else if (!(SCE_S_ISDIR(entryA->d_stat.st_mode)) && (SCE_S_ISDIR(entryB->d_stat.st_mode)))
		return 1;
		
	return sceLibcStrcasecmp(entryA->d_name, entryB->d_name);
}

char *Utils_Basename(const char *filename) {
	char *p = sceClibStrrchr (filename, '/');
	return p ? p + 1 : (char *) filename;
}

int power_tick_thread(SceSize args, void *argp) {
	while (SCE_TRUE) {
		sceKernelWaitEventFlag(event_flag_uid, FLAG_ELEVENMPVA_IS_POWER_LOCKED, SCE_KERNEL_EVF_WAITMODE_AND, NULL, NULL);
		sceKernelPowerTick(SCE_KERNEL_POWER_TICK_DISABLE_AUTO_SUSPEND);

		sceKernelDelayThread(10 * 1000 * 1000);
	}
	return 0;
}

void Utils_InitPowerTick(void) {
	SceUID thid = 0;
	if (R_SUCCEEDED(thid = sceKernelCreateThread("power_tick_thread", power_tick_thread, 191, 0x1000, 0, 0, NULL)))
		sceKernelStartThread(thid, 0, NULL);
}

SceBool Utils_IsDecoderUsed(void) {
	if (!sceKernelPollEventFlag(event_flag_uid, FLAG_ELEVENMPVA_IS_DECODER_USED, SCE_KERNEL_EVF_WAITMODE_AND, NULL))
		return SCE_TRUE;
	else
		return SCE_FALSE;
}

SceBool Utils_IsFinishedPlaylist(void) {
	if (!sceKernelPollEventFlag(event_flag_uid, FLAG_ELEVENMPVA_IS_FINISHED_PLAYLIST, SCE_KERNEL_EVF_WAITMODE_AND, NULL))
		return SCE_TRUE;
	else
		return SCE_FALSE;
}

void Utils_LoadIme(SceImeParam* param) {

	if (!ime_module_loaded) {
		sceSysmoduleLoadModule(SCE_SYSMODULE_IME);
		ime_module_loaded = SCE_TRUE;
	}

	sceImeParamInit(param);
	param->supportedLanguages = 0;
	param->languagesForced = SCE_FALSE;
	param->option = 0;
	param->filter = NULL;
	param->arg = NULL;
	param->work = libime_work;
}

void Utils_UnloadIme(void) {
	sceSysmoduleUnloadModule(SCE_SYSMODULE_IME);
	ime_module_loaded = SCE_FALSE;
}

void Utils_Utf8ToUtf16(SceWChar16* dst, char* src) {
	for (int i = 0; src[i];) {
		if ((src[i] & 0xE0) == 0xE0) {
			*(dst++) = ((src[i] & 0x0F) << 12) | ((src[i + 1] & 0x3F) << 6) | (src[i + 2] & 0x3F);
			i += 3;
		}
		else if ((src[i] & 0xC0) == 0xC0) {
			*(dst++) = ((src[i] & 0x1F) << 6) | (src[i + 1] & 0x3F);
			i += 2;
		}
		else {
			*(dst++) = src[i];
			i += 1;
		}
	}

	*dst = '\0';
}

void Utils_WriteSafeMem(void* data, SceSize buf_size,  SceOff offset) {
	sceAppUtilSaveSafeMemory(data, buf_size, offset);
}

void Utils_ReadSafeMem(void* buf, SceSize buf_size, SceOff offset) {
	sceAppUtilLoadSafeMemory(buf, buf_size, offset);
}
