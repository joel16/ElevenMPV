#include <psp2/kernel/clib.h>
#include <psp2/io/fcntl.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "config.h"
#include "fs.h"

#define CONFIG_VERSION 1

config_t config;
static int config_version_holder = 0;

extern void* mspace;

const char *config_file =
	"config_ver = %d\n"
	"metadata_flac = %d\n"
	"metadata_mp3 = %d\n"
	"metadata_opus = %d\n"
	"sort = %d\n"
	"alc_mode = %d\n"
	"device = %d";

int Config_Save(config_t config) {
	int ret = 0;
	
	char *buf = sceClibMspaceMalloc(mspace, 128);
	int len = sceClibSnprintf(buf, 128, config_file, CONFIG_VERSION, config.meta_flac, config.meta_mp3, config.meta_opus, config.sort, 
		config.alc_mode, config.device);
	
	if (R_FAILED(ret = FS_WriteFile("savedata0:config.cfg", buf, len))) {
		sceClibMspaceFree(mspace, buf);
		return ret;
	}
	
	sceClibMspaceFree(mspace, buf);
	return 0;
}	
	
int Config_Load(void) {
	int ret = 0;
	
	if (!FS_FileExists("savedata0:config.cfg")) {
		// set these to the following by default:
		config.meta_flac = SCE_FALSE;
		config.meta_mp3 = SCE_TRUE;
		config.meta_opus = SCE_TRUE;
		config.sort = 0;
		config.alc_mode = 0;
		config.device = 0;
		return Config_Save(config);
	}

	SceOff size = 0;
	FS_GetFileSize("savedata0:config.cfg", &size);
	char *buf = sceClibMspaceMalloc(mspace, size + 1);

	if (R_FAILED(ret = FS_ReadFile("savedata0:config.cfg", buf, size))) {
		sceClibMspaceFree(mspace, buf);
		return ret;
	}

	buf[size] = '\0';
	sscanf(buf, config_file, &config_version_holder, &config.meta_flac, &config.meta_mp3, &config.meta_opus, &config.sort, 
		&config.alc_mode, &config.device);
	sceClibMspaceFree(mspace, buf);

	// Delete config file if config file is updated. This will rarely happen.
	if (config_version_holder  < CONFIG_VERSION) {
		sceIoRemove("savedata0:config.cfg");
		config.meta_flac = SCE_FALSE;
		config.meta_mp3 = SCE_TRUE;
		config.meta_opus = SCE_TRUE;
		config.sort = 0;
		config.alc_mode = 0;
		config.device = 0;
		return Config_Save(config);
	}

	return 0;
}

int Config_GetLastDirectory(void) {
	int ret = 0;
	const char *root_paths[] = {
		"ux0:/",
		"ur0:/",
		"uma0:/"
	};
	
	if (!FS_FileExists("savedata0:lastdir.txt")) {
		sceClibSnprintf(root_path, 8, "ux0:/");
		FS_WriteFile("savedata0:lastdir.txt", root_path, strlen(root_path) + 1);
		strcpy(cwd, root_path); // Set Start Path to "sdmc:/" if lastDir.txt hasn't been created.
	}
	else {
		strcpy(root_path, root_paths[config.device]);
		SceOff size = 0;

		FS_GetFileSize("savedata0:lastdir.txt", &size);
		char *buf = sceClibMspaceMalloc(mspace, size + 1);

		if (R_FAILED(ret = FS_ReadFile("savedata0:lastdir.txt", buf, size))) {
			sceClibMspaceFree(mspace, buf);
			return ret;
		}

		buf[size] = '\0';
		char path[512];
		sscanf(buf, "%[^\n]s", path);
	
		if (FS_DirExists(path)) // Incase a directory previously visited had been deleted, set start path to sdmc:/ to avoid errors.
			strcpy(cwd, path);
		else
			strcpy(cwd, root_path);
		
		sceClibMspaceFree(mspace, buf);
	}
	
	return 0;
}
