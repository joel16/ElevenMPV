#ifndef _ELEVENMPV_AUDIO_MP3_H_
#define _ELEVENMPV_AUDIO_MP3_H_

#include <psp2/types.h>

int MP3_Init(const char *path);
SceUInt32 MP3_GetSampleRate(void);
SceUInt8 MP3_GetChannels(void);
void MP3_Decode(void *buf, unsigned int length, void *userdata);
SceUInt64 MP3_GetPosition(void);
SceUInt64 MP3_GetLength(void);
void MP3_Term(void);

#endif
