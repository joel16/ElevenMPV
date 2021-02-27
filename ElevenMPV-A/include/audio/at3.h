#ifndef _ELEVENMPV_AUDIO_AT3_H_
#define _ELEVENMPV_AUDIO_AT3_H_

#include <kernel.h>

typedef struct _SceIoPreadAsyncParam {
	SceOff offset;
	SceIoAsyncParam* asyncParam;
	int a3;
} _SceIoPreadAsyncParam;

SceUID _sceIoPreadAsync(SceUID fd, void *data, SceSize size, _SceIoPreadAsyncParam *param);

int AT3_Init(const char *path);
SceUInt32 AT3_GetSampleRate(void);
SceUInt8 AT3_GetChannels(void);
void AT3_Decode(void *buf, unsigned int length, void *userdata);
SceUInt64 AT3_GetPosition(void);
SceUInt64 AT3_GetLength(void);
SceUInt64 AT3_Seek(SceUInt64 index);
void AT3_Term(void);

#endif
