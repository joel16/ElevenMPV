#include <psp2/kernel/clib.h>
#include <psp2/kernel/iofilemgr.h>
#include <psp2/libc.h>

#include "common.h"
#include "config.h"
#include "utils.h"
#include "fs.h"

#define CONFIG_VERSION 12

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
	"power_saving = %d\n"
	"power_timer = %d\n"
	"notify_mode = %d\n"
	"device = %d";

int Config_Save(config_t config) {
	int ret = 0;
	
	char *buf = sceLibcMalloc(190);
	int len = sceClibSnprintf(buf, 190, config_file, CONFIG_VERSION, config.sort, config.alc_mode, config.eq_mode, config.eq_volume,
		config.motion_mode, config.motion_timer, config.motion_degree, config.power_saving, config.power_timer, config.notify_mode, config.device);
	
	if (R_FAILED(ret = FS_WriteFile("savedata0:config.cfg", buf, len))) {
		sceLibcFree(buf);
		return ret;
	}
	
	sceLibcFree(buf);
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
		config.power_saving = 1;
		config.power_timer = 1;
		config.notify_mode = 2;
		config.device = 0;
		return Config_Save(config);
	}

	SceOff size = 0;
	FS_GetFileSize("savedata0:config.cfg", &size);
	char *buf = sceLibcMalloc(size + 1);

	if (R_FAILED(ret = FS_ReadFile("savedata0:config.cfg", buf, size))) {
		sceLibcFree(buf);
		return ret;
	}

	buf[size] = '\0';
	sceLibcSscanf(buf, config_file, &config_version_holder, &config.sort, &config.alc_mode, &config.eq_mode,
		&config.eq_volume, &config.motion_mode, &config.motion_timer, &config.motion_degree, &config.power_saving,
		&config.power_timer, &config.notify_mode, &config.device);
	sceLibcFree(buf);

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
		"grw0:/"
	};
	
	int exist;
	Utils_ReadSafeMem((void *)&exist, 4, 0);

	if (!exist) {
		ret = sceClibSnprintf(root_path, 8, "ux0:/");
		Utils_WriteSafeMem((void*)&ret, 4, 0);
		Utils_WriteSafeMem((void*)root_path, ret, 4);
		sceLibcStrcpy(cwd, root_path); // Set Start Path to "sdmc:/" if lastDir.txt hasn't been created.
	}
	else {
		sceLibcStrcpy(root_path, root_paths[config.device]);

		char *buf = sceLibcMalloc(exist + 1);

		Utils_ReadSafeMem((void *)buf, exist, 4);

		buf[exist] = '\0';
		char path[512];
		sceLibcSscanf(buf, "%[^\n]s", path);
	
		if (FS_DirExists(path)) // Incase a directory previously visited had been deleted, set start path to sdmc:/ to avoid errors.
			sceLibcStrcpy(cwd, path);
		else
			sceLibcStrcpy(cwd, root_path);
		
		sceLibcFree(buf);
	}
	
	return 0;
}
