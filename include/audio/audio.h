#ifndef _ELEVENMPV_AUDIO_H_
#define _ELEVENMPV_AUDIO_H_

#include <psp2/types.h>
#include <vita2d.h>

extern SceBool playing, paused;

typedef struct {
	SceBool has_meta;
    char title[32];
    char album[32];
    char artist[32];
    char year[32];
    char comment[32];
    char genre[32];
    vita2d_texture *cover_image;
} Audio_Metadata;

extern Audio_Metadata metadata;

int Audio_Init(const char *path);
SceBool Audio_IsPaused(void);
void Audio_Pause(void);
void Audio_Stop(void);
SceUInt64 Audio_GetPosition(void);
SceUInt64 Audio_GetLength(void);
SceUInt64 Audio_GetPositionSeconds(void);
SceUInt64 Audio_GetLengthSeconds(void);
void Audio_Term(void);

#endif
