#include <psp2/io/fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "config.h"
#include "fs.h"

#define CONFIG_VERSION 1

config_t config;
static int config_version_holder = 0;

const char *config_file =
	"config_ver = %d\n"
	"metadata_flac = %d\n"
	"metadata_mp3 = %d\n"
	"metadata_opus = %d\n"
	"sort = %d\n"
	"alc_mode = %d";

int Config_Save(config_t config) {
	int ret = 0;
	
	char *buf = malloc(128);
	int len = snprintf(buf, 128, config_file, CONFIG_VERSION, config.meta_flac, config.meta_mp3, config.meta_opus, config.sort, config.alc_mode);
	
	if (R_FAILED(ret = FS_WriteFile("ux0:data/ElevenMPV/config.cfg", buf, len))) {
		free(buf);
		return ret;
	}
	
	free(buf);
	return 0;
}	
	
int Config_Load(void) {
	int ret = 0;
	
	if (!FS_FileExists("ux0:data/ElevenMPV/config.cfg")) {
		// set these to the following by default:
		config.meta_flac = SCE_FALSE;
		config.meta_mp3 = SCE_TRUE;
		config.meta_opus = SCE_TRUE;
		config.sort = 0;
		config.alc_mode = 0;
		return Config_Save(config);
	}

	SceOff size = 0;
	FS_GetFileSize("ux0:data/ElevenMPV/config.cfg", &size);
	char *buf = malloc(size + 1);

	if (R_FAILED(ret = FS_ReadFile("ux0:data/ElevenMPV/config.cfg", buf, size))) {
		free(buf);
		return ret;
	}

	buf[size] = '\0';
	sscanf(buf, config_file, &config_version_holder, &config.meta_flac, &config.meta_mp3, &config.meta_opus, &config.sort, &config.alc_mode);
	free(buf);

	// Delete config file if config file is updated. This will rarely happen.
	if (config_version_holder  < CONFIG_VERSION) {
		sceIoRemove("ux0:data/ElevenMPV/config.cfg");
		config.meta_flac = SCE_FALSE;
		config.meta_mp3 = SCE_TRUE;
		config.meta_opus = SCE_TRUE;
		config.sort = 0;
		config.alc_mode = 0;
		return Config_Save(config);
	}

	return 0;
}

int Config_GetLastDirectory(void) {
	int ret = 0;
	
	if (!FS_FileExists("ux0:data/ElevenMPV/lastdir.txt")) {
		FS_WriteFile("ux0:data/ElevenMPV/lastdir.txt", ROOT_PATH, strlen(ROOT_PATH) + 1);
		strcpy(cwd, ROOT_PATH); // Set Start Path to "sdmc:/" if lastDir.txt hasn't been created.
	}
	else {
		SceOff size = 0;

		FS_GetFileSize("ux0:data/ElevenMPV/lastdir.txt", &size);
		char *buf = malloc(size + 1);

		if (R_FAILED(ret = FS_ReadFile("ux0:data/ElevenMPV/lastdir.txt", buf, size))) {
			free(buf);
			return ret;
		}

		buf[size] = '\0';
		char path[512];
		sscanf(buf, "%[^\n]s", path);
	
		if (FS_DirExists(path)) // Incase a directory previously visited had been deleted, set start path to sdmc:/ to avoid errors.
			strcpy(cwd, path);
		else
			strcpy(cwd, ROOT_PATH);
		
		free(buf);
	}
	
	return 0;
}
