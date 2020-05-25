#ifndef _ELEVENMPV_AUDIO_AAC_H_
#define _ELEVENMPV_AUDIO_AAC_H_

#include <psp2/types.h>

int AAC_Init(const char *path);
SceUInt64 AAC_GetPosition(void);
SceUInt64 AAC_GetLength(void);
SceUInt64 AAC_Seek(SceUInt64 index);
void AAC_Term(void);

#endif
