#ifndef _ELEVENMPV_AUDIO_WAV_H_
#define _ELEVENMPV_AUDIO_WAV_H_

#include <psp2/types.h>

int WAV_Init(const char *path);
SceUInt32 WAV_GetSampleRate(void);
SceUInt8 WAV_GetChannels(void);
void WAV_Decode(void *buf, unsigned int length, void *userdata);
SceUInt64 WAV_GetPosition(void);
SceUInt64 WAV_GetLength(void);
void WAV_Term(void);

#endif
