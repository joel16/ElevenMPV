#ifndef _ELEVENMPV_AUDIO_MP3_H_
#define _ELEVENMPV_AUDIO_MP3_H_

#include <psp2/types.h>

int MP3_Init(const char *path);
SceUInt64 MP3_GetPosition(void);
SceUInt64 MP3_GetLength(void);
SceUInt64 MP3_Seek(SceUInt64 index);
void MP3_Term(void);

#endif
