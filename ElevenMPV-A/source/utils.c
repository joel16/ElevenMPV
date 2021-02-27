#include <apputil.h>
#include <kernel.h>
#include <system_param.h>
#include <notification_util.h>
#include <libsysmodule.h> 
#include <shellsvc.h>
#include <display.h>
#include <power.h> 
#include <libime.h> 
#include <appmgr.h> 
#include <shellaudio.h> 
#include <taihen.h>
#include <string.h>

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
int sceAppMgrAcquireBgmPortForMusicPlayer();
int sceAppMgrAcquireBgmPortWithPriority(int priority);

extern SceUID main_thread_uid;
extern SceUID event_flag_uid;

#ifdef DEBUG
extern SceAppMgrBudgetInfo budget_info;
#endif // DEBUG

static SceCtrlData pad, old_pad;
static SceBool ime_module_loaded = SCE_FALSE;
static SceBool is_deactivated = SCE_FALSE;
static SceBool keep_alive = SCE_FALSE;
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
		if (!sceKernelPollEventFlag(event_flag_uid, FLAG_ELEVENMPVA_IS_FG, SCE_KERNEL_EVF_WAITMODE_AND, NULL) || keep_alive) {

			sceClibMemset(&pad, 0, sizeof(SceCtrlData));
			sceCtrlPeekBufferPositive(0, &pad, 1);

			pressed = pad.buttons & ~old_pad.buttons;

			if (config.stick_skip) {
				if (pad.rx < 0x10) {
					pressed |= SCE_CTRL_L;
				}
				else if (pad.rx > 0xEF) {
					pressed |= SCE_CTRL_R;
				}
			}

			sceClibMemcpy(&old_pad, &pad, sizeof(SceCtrlData));

			//for controls update need to check every frame at 60fps
			sceDisplayWaitVblankStart();
		}
		else
			sceKernelDelayThread(10 * 1000);
	}

	return 0;
}

void Utils_Exit(void) {
	sceNotificationUtilCleanHistory();
	SceUID modid = taiLoadStartKernelModule("ux0:app/GRVA00008/module/kernel/exit_module.suprx", 0, NULL, 0);
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

void Utils_NotificationEventHandler(void *udata) {
	switch (config.notify_mode) {
	case 1:
		sceKernelSetEventFlag(event_flag_uid, FLAG_ELEVENMPVA_IS_FINISHED_PLAYLIST);
		break;
	case 2:
		Config_Save(config);
		if (!Utils_IsDecoderUsed()) {
			sceMusicPlayerServiceSendEvent(SCE_MUSICCORE_EVENTID_STOP, 0);
			sceMusicPlayerServiceTerminate();
		}
		Utils_Exit();
		break;
	}
}

void Utils_Activate(void) {
	if ((paused || !playing) && Utils_IsDecoderUsed())
		sceAppMgrAcquireBgmPortForMusicPlayer();
	else if ((paused || !playing) && !Utils_IsDecoderUsed())
		sceAppMgrAcquireBgmPortWithPriority(0x80);
	sceKernelSetEventFlag(event_flag_uid, FLAG_ELEVENMPVA_IS_FG);
	sceKernelChangeThreadPriority(main_thread_uid, 160);
}

void Utils_Deactivate(void) {
	if (paused || !playing)
		sceAppMgrReleaseBgmPort();
	sceKernelClearEventFlag(event_flag_uid, ~FLAG_ELEVENMPVA_IS_FG);
	sceKernelChangeThreadPriority(main_thread_uid, 251);
}

int Utils_PowerCallback(int notifyId, int notifyCount, int powerInfo, void *common)
{
	if (powerInfo & 0x400000) {
		if (!is_deactivated) {
			Utils_Deactivate();
			if (config.stick_skip)
				sceCtrlSetSamplingMode(SCE_CTRL_MODE_DIGITALANALOG);
			keep_alive = SCE_TRUE;
		}
	}
	else if (powerInfo & 0x800000) {
		if (!is_deactivated) {
			Utils_Activate();
			if (config.stick_skip)
				sceCtrlSetSamplingMode(SCE_CTRL_MODE_DIGITALONLY);
			keep_alive = SCE_FALSE;
		}
	}

	return 0;
}

int Utils_AppStatusWatchdog(SceSize argc, void* argv) {

	SceAppMgrEvent appEvent;

	SceUID shfb_id = vita2d_get_shfbid();
	SceSharedFbInfo info;
	sceSharedFbGetInfo(shfb_id, &info);

	SceUID cbid = sceKernelCreateCallback("power_cb", 0, Utils_PowerCallback, NULL);
	scePowerRegisterCallback(cbid);

	while (SCE_TRUE) {

		//keep alive for deactivated FG state

		if (keep_alive) {
			sceSharedFbBegin(shfb_id, &info);
			sceSharedFbEnd(shfb_id);
		}

		//check app status

		_sceAppMgrReceiveEvent(&appEvent);
		switch (appEvent.event) {
		case SCE_APP_EVENT_ON_ACTIVATE:
			Utils_Activate();
			is_deactivated = SCE_FALSE;
			break;
		case SCE_APP_EVENT_ON_DEACTIVATE:
			Utils_Deactivate();
			is_deactivated = SCE_TRUE;
			break;
		case SCE_APP_EVENT_REQUEST_QUIT:
			sceAppMgrReleaseBgmPort();
			Config_Save(config);
			if (!Utils_IsDecoderUsed()) {
				sceMusicPlayerServiceSendEvent(SCE_MUSICCORE_EVENTID_STOP, 0);
				sceMusicPlayerServiceTerminate();
			}
			Utils_Exit();
			break;
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
				Utils_Utf8ToUtf16(finish_param.text, "Playback has been stopped.");
				sceNotificationUtilProgressFinish(&finish_param);
				sceKernelClearEventFlag(event_flag_uid, ~FLAG_ELEVENMPVA_IS_END_NOTIFY);
			}
			else if (!sceKernelPollEventFlag(event_flag_uid, FLAG_ELEVENMPVA_IS_UPDATE_NOTIFY, SCE_KERNEL_EVF_WAITMODE_AND, NULL)) {
				sceNotificationUtilProgressUpdate(&notify_update_param);
				sceKernelClearEventFlag(event_flag_uid, ~FLAG_ELEVENMPVA_IS_UPDATE_NOTIFY);
			}
		}

		sceKernelDelayThreadCB(10 * 1000);
	}

	return 0;
}

//For nongame signed application
/*int Utils_AppStatusWatchdog(SceSize argc, void* argv) {

	SceAppMgrAppState app_state;
	SceAppMgrSystemEvent sys_event;

	SceUID shfb_id = vita2d_get_shfbid();
	SceSharedFbInfo info;
	sceSharedFbGetInfo(shfb_id, &info);

	SceUID cbid = sceKernelCreateCallback("power_cb", 0, Utils_PowerCallback, NULL);
	scePowerRegisterCallback(cbid);

	while (SCE_TRUE) {

		//keep alive for deactivated FG state

		if (keep_alive) {
			sceSharedFbBegin(shfb_id, &info);
			sceSharedFbEnd(shfb_id);
		}

		//check app status

		_sceAppMgrGetAppState(&app_state, sizeof(SceAppMgrAppState), SCE_PSP2_SDK_VERSION);
		if (app_state.systemEventNum != 0) {
			sceAppMgrReceiveSystemEvent(&sys_event);
			switch (sys_event.systemEvent) {
			case SCE_APP_EVENT_ON_ACTIVATE:
				Utils_Activate();
				is_deactivated = SCE_FALSE;
				break;
			case SCE_APP_EVENT_ON_DEACTIVATE:
				Utils_Deactivate();
				is_deactivated = SCE_TRUE;
				break;
			case SCE_APP_EVENT_REQUEST_QUIT:
				sceAppMgrReleaseBgmPort();
				Config_Save(config);
				if (!Utils_IsDecoderUsed()) {
					shellAudioSendCommandForMusicPlayer(SCE_SHELLAUDIO_STOP, 0);
					shellAudioFinishForMusicPlayer();
				}
				Utils_Exit();
				break;
			}
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

	sceKernelDelayThreadCB(10 * 1000);
	}

	return 0;
}*/

int Utils_InitAppUtil(void) {

	/* sceCtrl rules */

	SceCtrlRapidFireRule scroll_rule_up;
	sceClibMemset(&scroll_rule_up, 0, sizeof(SceCtrlRapidFireRule));

	scroll_rule_up.uiMask = SCE_CTRL_UP;
	scroll_rule_up.uiTrigger = SCE_CTRL_UP;
	scroll_rule_up.uiTarget = SCE_CTRL_UP;
	scroll_rule_up.uiDelay = 20;
	scroll_rule_up.uiMake = 2;
	scroll_rule_up.uiBreak = 2;

	sceCtrlSetRapidFire(0, 0, &scroll_rule_up);

	SceCtrlRapidFireRule scroll_rule_down;
	sceClibMemset(&scroll_rule_down, 0, sizeof(SceCtrlRapidFireRule));

	scroll_rule_down.uiMask = SCE_CTRL_DOWN;
	scroll_rule_down.uiTrigger = SCE_CTRL_DOWN;
	scroll_rule_down.uiTarget = SCE_CTRL_DOWN;
	scroll_rule_down.uiDelay = 20;
	scroll_rule_down.uiMake = 2;
	scroll_rule_down.uiBreak = 2;

	sceCtrlSetRapidFire(0, 1, &scroll_rule_down);

	sceSysmoduleLoadModule(SCE_SYSMODULE_NOTIFICATION_UTIL);

	sceClibMemset(&notify_init_param, 0, sizeof(SceNotificationUtilProgressInitParam));
	sceClibMemset(&notify_update_param, 0, sizeof(SceNotificationUtilProgressUpdateParam));

	SceUID status_watchdog = sceKernelCreateThread("app_status_watchdog", Utils_AppStatusWatchdog, 250, 0x1000, 0, 0, NULL);
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
	
	if ((SCE_STM_ISDIR(entryA->d_stat.st_mode)) && !(SCE_STM_ISDIR(entryB->d_stat.st_mode)))
		return -1;
	else if (!(SCE_STM_ISDIR(entryA->d_stat.st_mode)) && (SCE_STM_ISDIR(entryB->d_stat.st_mode)))
		return 1;
		
	return strcasecmp(entryA->d_name, entryB->d_name);
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
