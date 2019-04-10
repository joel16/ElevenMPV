#include <psp2/io/stat.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/shellutil.h>
#include <psp2/sysmodule.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "dirbrowse.h"
#include "fs.h"
#include "menu_displayfiles.h"
#include "textures.h"
#include "touch.h"
#include "utils.h"

static int GetLastDirectory(void) {
	int ret = 0;

	if (!FS_DirExists("ux0:data/elevenmpv"))
		sceIoMkdir("ux0:data/elevenmpv", 0777);

	if (!FS_FileExists("ux0:data/elevenmpv/lastdir.txt")) {
		char *buf = malloc(8);
		int length = snprintf(buf, 8, ROOT_PATH);

		if (R_FAILED(ret = FS_WriteFile("ux0:data/elevenmpv/lastdir.txt", buf, length))) {
			free(buf);
			return ret;
		}
		
		strcpy(cwd, ROOT_PATH);
	}
	else {
		SceOff size = 0;

		FS_GetFileSize("ux0:data/elevenmpv/lastdir.txt", &size);
		char *buf = malloc(size + 1);

		if (R_FAILED(ret = FS_ReadFile("ux0:data/elevenmpv/lastdir.txt", buf, size))) {
			free(buf);
			return ret;
		}

		buf[size] = '\0';
		char path[513];
		sscanf(buf, "%[^\n]s", path);
	
		if (FS_DirExists(path)) {
			if (R_SUCCEEDED(Dirbrowse_PopulateFiles(SCE_FALSE)))
				strcpy(cwd, path);
			else
				strcpy(cwd, ROOT_PATH);
		}
		else
			strcpy(cwd, ROOT_PATH);
		
		free(buf);
	}
	
	return 0;
}

int main(int argc, char *argv[]) {
	vita2d_init();
	font = vita2d_load_font_file("app0:Roboto-Regular.ttf");
	Textures_Load();

	Utils_InitAppUtil();
	SCE_CTRL_ENTER = Utils_GetEnterButton();
	SCE_CTRL_CANCEL = Utils_GetCancelButton();

	GetLastDirectory();
	Touch_Init();

	sceShellUtilInitEvents(0);
	sceSysmoduleLoadModule(SCE_SYSMODULE_MUSIC_EXPORT);
	Utils_InitPowerTick();

	Menu_DisplayFiles();

	sceSysmoduleUnloadModule(SCE_SYSMODULE_MUSIC_EXPORT);

	Touch_Shutdown();
	Utils_TermAppUtil();

	Textures_Free();
	vita2d_free_font(font);
	vita2d_fini();

	sceKernelExitProcess(0);
	return 0;
}
