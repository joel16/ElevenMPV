#ifndef _ELEVENMPV_CONFIG_H_
#define _ELEVENMPV_CONFIG_H_

#include <psp2/types.h>

typedef struct {
	SceBool meta_flac;
	SceBool meta_mp3;
	SceBool meta_opus;
	int sort;
	SceBool alc_mode;
	int eq_mode;
	SceBool motion_mode;
	int motion_timer;
	int motion_degree;
	SceBool power_saving;
	int power_timer;
	int notify_mode;
	int device;
} config_t;

extern config_t config;

int Config_Save(config_t config);
int Config_Load(void);
int Config_GetLastDirectory(void);

#endif
