#ifndef _ELEVENMPV_AUDIO_AT9_H_
#define _ELEVENMPV_AUDIO_AT9_H_

#include <psp2/types.h>

int AT9_Init(const char *path);
SceUInt64 AT9_GetPosition(void);
SceUInt64 AT9_GetLength(void);
SceUInt64 AT9_Seek(SceUInt64 index);
void AT9_Term(void);

#endif
