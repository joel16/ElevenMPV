#ifndef _ELEVENMPV_AUDIO_H_
#define _ELEVENMPV_AUDIO_H_

#include <psp2/types.h>
#include <vita2d_sys.h>

extern SceBool playing, paused;

#define MAX_IMAGE_WIDTH		512
#define MAX_IMAGE_HEIGHT	512
#define IMAGE_BUF_SIZE	(MAX_IMAGE_WIDTH * MAX_IMAGE_HEIGHT * 3)
#define JPEG_BUF_SIZE	(MAX_IMAGE_WIDTH * MAX_IMAGE_HEIGHT)

typedef struct {
	SceBool has_meta;
    char title[260];
    char album[64];
    char artist[260];
    char year[64];
    char comment[64];
    char genre[64];
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
SceUInt64 Audio_Seek(void);
void Audio_SetSeekPosition(SceUInt64 index);
void Audio_SetSeekMode(SceBool mode);
SceBool Audio_GetSeekMode(void);
void Audio_Term(void);

#endif
