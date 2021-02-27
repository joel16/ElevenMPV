#include <apputil.h>
#include <kernel.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "common.h"
#include "config.h"
#include "utils.h"
#include "fs.h"

#define CONFIG_VERSION 13

config_t config;
static int config_version_holder = 0;

const char *config_file =
	"config_ver = %d\n"
	"sort = %d\n"
	"alc_mode = %d\n"
	"eq_mode = %d\n"
	"eq_volume = %d\n"
	"motion_mode = %d\n"
	"motion_timer = %d\n"
	"motion_degree = %d\n"
	"stick_skip = %d\n"
	"power_saving = %d\n"
	"power_timer = %d\n"
	"notify_mode = %d\n"
	"device = %d";

int Config_Save(config_t config) {
	int ret = 0;
	SceSize reqSize;
	
	char *buf = malloc(190);
	int len = sceClibSnprintf(buf, 190, config_file, CONFIG_VERSION, config.sort, config.alc_mode, config.eq_mode, config.eq_volume,
		config.motion_mode, config.motion_timer, config.motion_degree, config.stick_skip, config.power_saving, config.power_timer, config.notify_mode, config.device);
	
	SceAppUtilSaveDataDataSaveItem item;
	sceClibMemset(&item, 0, sizeof(SceAppUtilSaveDataDataSaveItem));
	item.dataPath = "config.cfg";
	item.buf = buf;
	item.bufSize = len;
	item.mode = SCE_APPUTIL_SAVEDATA_DATA_SAVE_MODE_FILE;

	SceAppUtilMountPoint mount;
	sceClibStrncpy(mount.data, "savedata0:", SCE_APPUTIL_MOUNTPOINT_DATA_MAXSIZE);

	if (R_FAILED(ret = sceAppUtilSaveDataDataSave(NULL, &item, 1, &mount, &reqSize))) {
		sceClibPrintf("FS error: 0x%X\n", ret);
		free(buf);
		return ret;
	}
	
	free(buf);
	return 0;
}	
	
int Config_Load(void) {
	int ret = 0;
	
	if (!FS_FileExists("savedata0:config.cfg")) {
		// set these to the following by default:
		config.sort = 0;
		config.alc_mode = 0;
		config.eq_mode = 0;
		config.eq_volume = 0;
		config.motion_mode = 0;
		config.motion_timer = 3;
		config.motion_degree = 45;
		config.stick_skip = 1;
		config.power_saving = 1;
		config.power_timer = 1;
		config.notify_mode = 2;
		config.device = 0;
		return Config_Save(config);
	}

	SceOff size = 0;
	FS_GetFileSize("savedata0:config.cfg", &size);
	char *buf = malloc((size_t)(size + 1));

	if (R_FAILED(ret = FS_ReadFile("savedata0:config.cfg", buf, (int)size))) {
		free(buf);
		return ret;
	}

	buf[size] = '\0';
	sscanf(buf, config_file, &config_version_holder, &config.sort, &config.alc_mode, &config.eq_mode,
		&config.eq_volume, &config.motion_mode, &config.motion_timer, &config.motion_degree, &config.stick_skip,
		&config.power_saving, &config.power_timer, &config.notify_mode, &config.device);
	free(buf);

	// Delete config file if config file is updated. This will rarely happen.
	if (config_version_holder  < CONFIG_VERSION) {
		sceIoRemove("savedata0:config.cfg");
		config.sort = 0;
		config.alc_mode = 0;
		config.eq_mode = 0;
		config.eq_volume = 0;
		config.motion_mode = 0;
		config.motion_timer = 3;
		config.motion_degree = 45;
		config.stick_skip = 1;
		config.power_saving = 1;
		config.power_timer = 1;
		config.notify_mode = 2;
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
		"uma0:/",
		"xmc0:/",
		"imc0:/",
		"grw0:/"
	};
	
	int exist;
	Utils_ReadSafeMem((void *)&exist, 4, 0);

	if (!exist) {
		ret = sceClibSnprintf(root_path, 8, "ux0:/");
		Utils_WriteSafeMem((void*)&ret, 4, 0);
		Utils_WriteSafeMem((void*)root_path, ret, 4);
		strcpy(cwd, root_path); // Set Start Path to "sdmc:/" if lastDir.txt hasn't been created.
	}
	else {
		strcpy(root_path, root_paths[config.device]);

		char *buf = malloc(exist + 1);

		Utils_ReadSafeMem((void *)buf, exist, 4);

		buf[exist] = '\0';
		char path[512];
		sscanf(buf, "%[^\n]s", path);
	
		if (FS_DirExists(path)) // Incase a directory previously visited had been deleted, set start path to sdmc:/ to avoid errors.
			strcpy(cwd, path);
		else
			strcpy(cwd, root_path);
		
		free(buf);
	}
	
	return 0;
}
