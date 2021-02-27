#include <appmgr.h>
#include <kernel.h>
#include <shellsvc.h>
#include <libsysmodule.h>
#include <font/libpvf.h>
#include <gxm.h>
#include <libdbg.h>
#include <shellaudio.h>

#include "common.h"
#include "config.h"
#include "dirbrowse.h"
#include "fs.h"
#include "menu_displayfiles.h"
#include "textures.h"
#include "touch_e.h"
#include "utils.h"

int sceLibcHeapSize = 1 * 1024 * 1024;

SceUID main_thread_uid;
SceUID event_flag_uid;

#ifdef DEBUG
SceAppMgrBudgetInfo budget_info;
#endif

int isKoreanChar(unsigned int c) 
{
	unsigned short ch = c;

	// Hangul compatibility jamo block
	if (0x3130 <= ch && ch <= 0x318F) {
		return 1;
	}

	// Hangul syllables block
	if (0xAC00 <= ch && ch <= 0xD7AF) {
		return 1;
	}

	// Korean won sign
	if (ch == 0xFFE6) {
		return 1;
	}

	return 0;
}

int main() {

	sceDbgSetMinimumLogLevel(SCE_DBG_LOG_LEVEL_ERROR);

	//Grow memory if possible
	sceAppMgrGrowMemory3(16 * 1024 * 1024, 1);

#ifdef DEBUG
	sceClibMemset(&budget_info, 0, sizeof(SceAppMgrBudgetInfo));
	budget_info.size = 0x88;
	sceAppMgrGetBudgetInfo(&budget_info);
	sceClibPrintf("----- EMPA-A INITIAL BUDGET -----");
	sceClibPrintf("LPDDR2: %d MB\n", budget_info.freeLPDDR2 / 1024 / 1024);
#endif

	main_thread_uid = sceKernelGetThreadId();
	sceKernelChangeThreadPriority(main_thread_uid, 160);

	//Reset repeat state
	sceMusicPlayerServiceInitialize(0);
	sceMusicPlayerServiceSetRepeatMode(SCE_MUSICSERVICE_REPEAT_DISABLE);
	sceMusicPlayerServiceTerminate();

	sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_AUDIOCODEC);

	event_flag_uid = sceKernelCreateEventFlag("ElevenMPVA_thread_event_flag", SCE_KERNEL_ATTR_MULTI, FLAG_ELEVENMPVA_IS_FG | FLAG_ELEVENMPVA_IS_DECODER_USED, NULL);

	vita2d_init_param v2d_param;
	sceClibMemset(&v2d_param, 0, sizeof(vita2d_init_param));
	v2d_param.heap_size = 512 * 1024;
	v2d_param.temp_pool_size = 512 * 1024;
	v2d_param.vdm_ring_buffer_size = 4 * 1024;
	v2d_param.vertex_ring_buffer_size = 128 * 1024;
	v2d_param.fragment_ring_buffer_size = 64 * 1024;
	v2d_param.fragment_usse_ring_buffer_size = 4 * 1024;
	v2d_param.vdm_ring_buffer_attrib = VITA2D_MEM_ATTRIB_SHARED;
	v2d_param.vertex_ring_buffer_attrib = VITA2D_MEM_ATTRIB_SHARED;
	v2d_param.fragment_ring_buffer_attrib = VITA2D_MEM_ATTRIB_SHARED;
	v2d_param.fragment_usse_ring_buffer_attrib = VITA2D_MEM_ATTRIB_SHARED;

	vita2d_init(&v2d_param);
	vita2d_set_vblank_wait(0);

	vita2d_JPEG_ARM_decoder_initialize();

	vita2d_system_pvf_config configs[] = {
	{ SCE_PVF_LANGUAGE_K, SCE_PVF_FAMILY_SANSERIF, SCE_PVF_STYLE_REGULAR, isKoreanChar },
	{ SCE_PVF_LANGUAGE_J, SCE_PVF_FAMILY_SANSERIF, SCE_PVF_STYLE_REGULAR, NULL },
	};

	font = vita2d_load_system_shared_pvf(2, configs, 13, 13);

	Textures_Load();

	Utils_InitAppUtil();

	Config_Load();
	Config_GetLastDirectory();

	SCE_CTRL_ENTER = Utils_GetEnterButton();
	SCE_CTRL_CANCEL = Utils_GetCancelButton();

	Touch_Init();

	sceShellUtilInitEvents(0);
	Utils_InitPowerTick();

	Menu_DisplayFiles();

	return 0;
}
