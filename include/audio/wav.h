#ifndef _ELEVENMPV_AUDIO_WAV_H_
#define _ELEVENMPV_AUDIO_WAV_H_

#include <psp2/types.h>

int WAV_Init(const char *path);
SceUInt64 WAV_GetPosition(void);
SceUInt64 WAV_GetLength(void);
SceUInt64 WAV_Seek(SceUInt64 index);
void WAV_Term(void);

#endif
