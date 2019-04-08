#ifndef _ELEVENMPV_AUDIO_OPUS_H_
#define _ELEVENMPV_AUDIO_OPUS_H_

#include <psp2/types.h>

int OPUS_Init(const char *path);
SceUInt32 OPUS_GetSampleRate(void);
SceUInt8 OPUS_GetChannels(void);
void OPUS_Decode(void *buf, unsigned int length, void *userdata);
SceUInt64 OPUS_GetPosition(void);
SceUInt64 OPUS_GetLength(void);
void OPUS_Term(void);

#endif
