#ifndef _ELEVENMPV_AUDIO_FLAC_H_
#define _ELEVENMPV_AUDIO_FLAC_H_

#include <psp2/types.h>

int FLAC_Init(const char *path);
SceUInt32 FLAC_GetSampleRate(void);
SceUInt8 FLAC_GetChannels(void);
void FLAC_Decode(void *buf, unsigned int length, void *userdata);
SceUInt64 FLAC_GetPosition(void);
SceUInt64 FLAC_GetLength(void);
void FLAC_Term(void);

#endif
