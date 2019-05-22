#ifndef _ELEVENMPV_CONFIG_H_
#define _ELEVENMPV_CONFIG_H_

#include <psp2/types.h>

typedef struct {
	SceBool meta_flac;
	SceBool meta_mp3;
	SceBool meta_opus;
	int sort;
	int alc_mode;
} config_t;

extern config_t config;

int Config_Save(config_t config);
int Config_Load(void);
int Config_GetLastDirectory(void);

#endif
