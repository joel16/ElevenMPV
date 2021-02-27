#ifndef _ELEVENMPV_AUDIO_SH_COMMON_H_
#define _ELEVENMPV_AUDIO_SH_COMMON_H_

int SHC_Init(const char *path);
SceUInt64 SHC_GetPosition(void);
SceUInt64 SHC_GetLength(void);
SceUInt64 SHC_Seek(SceUInt64 index);
void SHC_Term(void);

#endif