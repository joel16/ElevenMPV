#include <psp2/appmgr.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/modulemgr.h> 
#include <psp2/kernel/clib.h>
#include <psp2/shellutil.h>
#include <psp2/sysmodule.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "config.h"
#include "dirbrowse.h"
#include "fs.h"
#include "menu_displayfiles.h"
#include "textures.h"
#include "touch.h"
#include "utils.h"

#define CLIB_HEAP_SIZE 1024 * 1024

int _newlib_heap_size_user = 1024 * 1024;
void* mspace;

int sceAppMgrAcquireBgmPortForMusicPlayer(void);

int main(int argc, char *argv[]) {

	void* clibm_base;
	SceUID clib_heap = sceKernelAllocMemBlock("ClibHeap", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE, CLIB_HEAP_SIZE, NULL);
	sceKernelGetMemBlockBase(clib_heap, &clibm_base);
	mspace = sceClibMspaceCreate(clibm_base, CLIB_HEAP_SIZE);

	sceKernelLoadStartModule("vs0:sys/external/libc.suprx", 0, NULL, 0, NULL, NULL);

	vita2d_clib_pass_mspace(mspace);
	vita2d_init();
	font = vita2d_load_font_file("app0:Roboto-Regular.ttf");
	Textures_Load();

	Config_Load();
	Config_GetLastDirectory();

	Utils_InitAppUtil();
	SCE_CTRL_ENTER = Utils_GetEnterButton();
	SCE_CTRL_CANCEL = Utils_GetCancelButton();

	sceAppMgrAcquireBgmPortForMusicPlayer();

	Touch_Init();

	sceShellUtilInitEvents(0);
	Utils_InitPowerTick();

	Menu_DisplayFiles();

	Touch_Shutdown();
	sceAppMgrReleaseBgmPort();
	Utils_TermAppUtil();

	Textures_Free();
	vita2d_free_font(font);
	vita2d_fini();

	sceKernelExitProcess(0);
	return 0;
}
