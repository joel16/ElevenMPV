#ifndef _ELEVENMPV_UTILS_H_
#define _ELEVENMPV_UTILS_H_

#include <psp2/ime.h>
#include <psp2/notificationutil.h>
#include <psp2/kernel/iofilemgr.h>

#define JPEGDEC_SIZE_LIMIT 262144 //500x500

#define SCE_KERNEL_ATTR_MULTI (0x00001000U)

#define SCE_KERNEL_EVF_WAITMODE_AND			(0x00000000U)
#define SCE_KERNEL_EVF_WAITMODE_OR			(0x00000001U)
#define SCE_KERNEL_EVF_WAITMODE_CLEAR_ALL	(0x00000002U)
#define SCE_KERNEL_EVF_WAITMODE_CLEAR_PAT	(0x00000004U)

#define SCE_APP_EVENT_UNK0					(0x00000003)
#define SCE_APP_EVENT_ON_ACTIVATE			(0x10000001)
#define SCE_APP_EVENT_ON_DEACTIVATE			(0x10000002)
#define SCE_APP_EVENT_UNK1					(0x10000300)
#define SCE_APP_EVENT_REQUEST_QUIT			(0x20000001)
#define SCE_APP_EVENT_UNK2					(0x30000003)

#define FLAG_ELEVENMPVA_IS_FG 1
#define FLAG_ELEVENMPVA_IS_DECODER_USED 2
#define FLAG_ELEVENMPVA_IS_POWER_LOCKED 4
#define FLAG_ELEVENMPVA_IS_FINISHED_PLAYLIST 8
#define FLAG_ELEVENMPVA_IS_READY_NOTIFY 16
#define FLAG_ELEVENMPVA_IS_END_NOTIFY 32
#define FLAG_ELEVENMPVA_IS_UPDATE_NOTIFY 64

void Utils_SetMax(int *set, int value, int max);
void Utils_SetMin(int *set, int value, int min);
int Utils_InitAppUtil(void);
int Utils_TermAppUtil(void);
int Utils_GetEnterButton(void);
int Utils_GetCancelButton(void);
int Utils_Alphasort(const void *p1, const void *p2);
char *Utils_Basename(const char *filename);
void Utils_InitPowerTick(void);
SceBool Utils_IsDecoderUsed(void);
SceBool Utils_IsFinishedPlaylist(void);
void Utils_LoadIme(SceImeParam* param);
void Utils_UnloadIme(void);
void Utils_Utf8ToUtf16(SceWChar16* dst, char* src);
void Utils_NotificationEventHandler(int a1);
void Utils_NotificationEnd(void);
void Utils_NotificationProgressBegin(SceNotificationUtilProgressInitParam* init_param);
void Utils_NotificationProgressUpdate(SceNotificationUtilProgressUpdateParam* update_param);
void Utils_WriteSafeMem(void* data, SceSize buf_size, SceOff offset);
void Utils_ReadSafeMem(void* buf, SceSize buf_size, SceOff offset);

#endif
