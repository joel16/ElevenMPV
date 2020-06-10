#include <psp2/appmgr.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/modulemgr.h> 
#include <psp2/kernel/clib.h>
#include <psp2/shellutil.h>
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

#define CLIB_HEAP_SIZE 1 * 1024 * 1024
int _newlib_heap_size_user = 3 * 1024 * 1024;

void* mspace;
SceUID main_thread_uid;
SceUID event_flag_uid;

int main(int argc, char *argv[]) {

	void* clibm_base;
	SceUID clib_heap = sceKernelAllocMemBlock("ClibHeap", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE, CLIB_HEAP_SIZE, NULL);
	sceKernelGetMemBlockBase(clib_heap, &clibm_base);
	mspace = sceClibMspaceCreate(clibm_base, CLIB_HEAP_SIZE);

	main_thread_uid = sceKernelGetThreadId();
	sceKernelChangeThreadPriority(main_thread_uid, 160);
	event_flag_uid = sceKernelCreateEventFlag("ElevenMPVA_thrread_event_flag", SCE_KERNEL_ATTR_MULTI, FLAG_ELEVENMPVA_IS_FG, NULL);

	sceKernelLoadStartModule("vs0:sys/external/libc.suprx", 0, NULL, 0, NULL, NULL);

	vita2d_clib_pass_mspace(mspace);
	vita2d_init();
	vita2d_set_vblank_wait(0);

	vita2d_system_pvf_config configs[] = {
		{SCE_PVF_LANGUAGE_LATIN, SCE_PVF_FAMILY_SANSERIF, SCE_PVF_STYLE_REGULAR, NULL},
	};

	font = vita2d_load_system_pvf(1, configs, 13, 13);

	Textures_Load();

	Config_Load();
	Config_GetLastDirectory();

	Utils_InitAppUtil();
	SCE_CTRL_ENTER = Utils_GetEnterButton();
	SCE_CTRL_CANCEL = Utils_GetCancelButton();

	Touch_Init();

	sceShellUtilInitEvents(0);
	Utils_InitPowerTick();

	Menu_DisplayFiles();

	return 0;
}
