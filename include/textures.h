#ifndef _ELEVENMPV_TEXTURES_H_
#define _ELEVENMPV_TEXTURES_H_

#include <vita2d_sys.h>
#include <psp2/io/stat.h> 

#define BTN_TOPBAR_Y 54
#define BTN_BACK_X 10

#define MAX_PATH_LENGTH 1024
#define MAX_NAME_LENGTH 256
#define MAX_SHORT_NAME_LENGTH 64

#define SCE_FIOS_FH_SIZE 80
#define SCE_FIOS_DH_SIZE 80
#define SCE_FIOS_OP_SIZE 168
#define SCE_FIOS_CHUNK_SIZE 64

#define SCE_FIOS_ALIGN_UP(val, align) (((val) + ((align) - 1)) & ~((align) - 1))
#define SCE_FIOS_STORAGE_SIZE(num, size) (((num) * (size)) + SCE_FIOS_ALIGN_UP(SCE_FIOS_ALIGN_UP((num), 8) / 8, 8))

#define SCE_FIOS_DH_STORAGE_SIZE(numDHs, pathMax) SCE_FIOS_STORAGE_SIZE(numDHs, SCE_FIOS_DH_SIZE + pathMax)
#define SCE_FIOS_FH_STORAGE_SIZE(numFHs, pathMax) SCE_FIOS_STORAGE_SIZE(numFHs, SCE_FIOS_FH_SIZE + pathMax)
#define SCE_FIOS_OP_STORAGE_SIZE(numOps, pathMax) SCE_FIOS_STORAGE_SIZE(numOps, SCE_FIOS_OP_SIZE + pathMax)
#define SCE_FIOS_CHUNK_STORAGE_SIZE(numChunks) SCE_FIOS_STORAGE_SIZE(numChunks, SCE_FIOS_CHUNK_SIZE)

#define SCE_FIOS_BUFFER_INITIALIZER  { 0, 0 }
#define SCE_FIOS_PSARC_DEARCHIVER_CONTEXT_INITIALIZER { sizeof(SceFiosPsarcDearchiverContext), 0, 0, 0, {0, 0, 0} }
#define SCE_FIOS_RAM_CACHE_CONTEXT_INITIALIZER { sizeof(SceFiosRamCacheContext), 0, (64 * 1024), NULL, NULL, 0, {0,0,0} }
#define SCE_FIOS_PARAMS_INITIALIZER { 0, sizeof(SceFiosParams), 0, 0, 2, 1, 0, 0, 256 * 1024, 2, 0, 0, 0, 0, 0, SCE_FIOS_BUFFER_INITIALIZER, SCE_FIOS_BUFFER_INITIALIZER, SCE_FIOS_BUFFER_INITIALIZER, SCE_FIOS_BUFFER_INITIALIZER, NULL, NULL, NULL, { 66, 189, 66 }, { 0x00000, 0x00000, 0x00000}, { 8 * 1024, 16 * 1024, 8 * 1024}}

typedef SceInt32 SceFiosFH;
typedef SceInt32 SceFiosDH;
typedef SceUInt64 SceFiosDate;
typedef SceInt64 SceFiosOffset;
typedef SceInt64 SceFiosSize;

typedef struct SceFiosPsarcDearchiverContext {
	SceSize sizeOfContext;
	SceSize workBufferSize;
	ScePVoid pWorkBuffer;
	SceIntPtr flags;
	SceIntPtr reserved[3];
} SceFiosPsarcDearchiverContext;

typedef struct SceFiosBuffer {
	ScePVoid pPtr;
	SceSize length;
} SceFiosBuffer;

typedef struct SceFiosParams {
	SceUInt32 initialized : 1;
	SceUInt32 paramsSize : 15;
	SceUInt32 pathMax : 16;
	SceUInt32 profiling;
	SceUInt32 ioThreadCount;
	SceUInt32 threadsPerScheduler;
	SceUInt32 extraFlag1 : 1;
	SceUInt32 extraFlags : 31;
	SceUInt32 maxChunk;
	SceUInt8 maxDecompressorThreadCount;
	SceUInt8 reserved1;
	SceUInt8 reserved2;
	SceUInt8 reserved3;
	SceIntPtr reserved4;
	SceIntPtr reserved5;
	SceFiosBuffer opStorage;
	SceFiosBuffer fhStorage;
	SceFiosBuffer dhStorage;
	SceFiosBuffer chunkStorage;
	ScePVoid pVprintf;
	ScePVoid pMemcpy;
	ScePVoid pProfileCallback;
	SceInt32 threadPriority[3];
	SceInt32 threadAffinity[3];
	SceInt32 threadStackSize[3];
} SceFiosParams;

typedef struct SceFiosRamCacheContext
{
	SceSize sizeOfContext;
	SceSize workBufferSize;
	SceSize blockSize;
	ScePVoid pWorkBuffer;
	SceName pPath;
	SceIntPtr flags;
	SceIntPtr reserved[3];
} SceFiosRamCacheContext;

vita2d_texture *icon_dir, *icon_file, *icon_audio, *btn_play, *btn_pause, *btn_rewind, *btn_forward, *btn_repeat, \
	*btn_shuffle, *icon_back, *toggle_on, *toggle_off, *radio_on, *radio_off;

void Textures_Load(void);
void Textures_Free(void);

#endif
