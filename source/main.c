#include <psp2/appmgr.h>
#include <psp2/kernel/iofilemgr.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/modulemgr.h> 
#include <psp2/kernel/clib.h>
#include <psp2/shellsvc.h>
#include <psp2/sysmodule.h>
#include <psp2/pvf.h>

#include "common.h"
#include "config.h"
#include "dirbrowse.h"
#include "fs.h"
#include "menu_displayfiles.h"
#include "textures.h"
#include "touch.h"
#include "utils.h"
#include "psp2_compat.h"
//#include "fios2_common.h"

SceUID main_thread_uid;
SceUID event_flag_uid;

#ifdef DEBUG
SceAppMgrBudgetInfo budget_info;
#endif

void _start(int argc, char *argv[]) {

#ifdef DEBUG
	sceClibMemset(&budget_info, 0, sizeof(SceAppMgrBudgetInfo));
	budget_info.size = 0x88;
	sceAppMgrGetBudgetInfo(&budget_info);
	sceClibPrintf("----- EMPA-A INITIAL BUDGET -----");
	sceClibPrintf("LPDDR2: %d MB\n", budget_info.freeLPDDR2 / 1024 / 1024);
#endif

	psp2CompatInit();
	//fios2Init();

	main_thread_uid = sceKernelGetThreadId();
	sceKernelChangeThreadPriority(main_thread_uid, 160);

	sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_AUDIOCODEC);

	event_flag_uid = sceKernelCreateEventFlag("ElevenMPVA_thread_event_flag", SCE_KERNEL_ATTR_MULTI, FLAG_ELEVENMPVA_IS_FG | FLAG_ELEVENMPVA_IS_DECODER_USED, NULL);

	vita2d_init_with_msaa_and_memsize(0, 4 * 1024, 128 * 1024, 64 * 1024, 4 * 1024, 0);
	vita2d_set_vblank_wait(0);

	vita2d_system_pvf_config config;
	sceClibMemset(&config, 0, sizeof(vita2d_system_pvf_config));
	config.language = SCE_PVF_LANGUAGE_CJK;
	config.family = SCE_PVF_FAMILY_SANSERIF;
	config.style = SCE_PVF_STYLE_REGULAR;
	font = vita2d_load_system_pvf(1, &config, 13, 13);

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
}
