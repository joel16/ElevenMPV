#include <psp2/io/stat.h> 
#include <psp2/kernel/clib.h>
#include <psp2/io/fcntl.h> 
#include <psp2/sysmodule.h>

#include "textures.h"

#define RAMCACHEBLOCKSIZE 64 * 1024

SceInt32 sceFiosInitialize(const SceFiosParams* params);
void sceFiosTerminate();

SceFiosSize sceFiosArchiveGetMountBufferSizeSync(const ScePVoid attr, const SceName path, ScePVoid params);
SceInt32 sceFiosArchiveMountSync(const ScePVoid attr, SceFiosFH* fh, const SceName path, const SceName mount_point, SceFiosBuffer mount_buffer, ScePVoid params);
SceInt32 sceFiosArchiveUnmountSync(const ScePVoid attr, SceFiosFH fh);

SceInt32 sceFiosIOFilterAdd(SceInt32 index, SceVoid(*callback)(), ScePVoid context);
SceInt32 sceFiosIOFilterRemove(SceInt32 index);

void sceFiosIOFilterPsarcDearchiver();
void sceFiosIOFilterCache();

static SceInt64 s_op_storage[SCE_FIOS_OP_STORAGE_SIZE(64, MAX_PATH_LENGTH) / sizeof(SceInt64) + 1];
static SceInt64 s_chunk_storage[SCE_FIOS_CHUNK_STORAGE_SIZE(1024) / sizeof(SceInt64) + 1];
static SceInt64 s_fh_storage[SCE_FIOS_FH_STORAGE_SIZE(32, MAX_PATH_LENGTH) / sizeof(SceInt64) + 1];
static SceInt64 s_dh_storage[SCE_FIOS_DH_STORAGE_SIZE(32, MAX_PATH_LENGTH) / sizeof(SceInt64) + 1];

static SceFiosPsarcDearchiverContext s_dearchiver_context = SCE_FIOS_PSARC_DEARCHIVER_CONTEXT_INITIALIZER;
static SceByte s_dearchiver_work_buffer[3 * 64 * 1024] __attribute__((aligned(64)));
static SceInt32 s_archive_index = 0;
static SceFiosBuffer s_mount_buffer = SCE_FIOS_BUFFER_INITIALIZER;
static SceFiosFH s_archive_fh = -1;

SceFiosRamCacheContext s_ramcache_context = SCE_FIOS_RAM_CACHE_CONTEXT_INITIALIZER;
static SceByte s_ramcache_work_buffer[10 * (64 * 1024)] __attribute__((aligned(8)));

extern void* mspace;

/* memcpy replacement for initializers */

ScePVoid memcpy(ScePVoid destination, const ScePVoid source, SceSize num)
{
	return sceClibMemcpy(destination, source, num);
}

void Textures_Load(void) {

	SceFiosParams params = SCE_FIOS_PARAMS_INITIALIZER;
	params.opStorage.pPtr = s_op_storage;
	params.opStorage.length = sizeof(s_op_storage);
	params.chunkStorage.pPtr = s_chunk_storage;
	params.chunkStorage.length = sizeof(s_chunk_storage);
	params.fhStorage.pPtr = s_fh_storage;
	params.fhStorage.length = sizeof(s_fh_storage);
	params.dhStorage.pPtr = s_dh_storage;
	params.dhStorage.length = sizeof(s_dh_storage);
	params.pathMax = MAX_PATH_LENGTH;

	sceFiosInitialize(&params);

	/* dearchiver overlay */

	s_dearchiver_context.workBufferSize = sizeof(s_dearchiver_work_buffer);
	s_dearchiver_context.pWorkBuffer = s_dearchiver_work_buffer;
	sceFiosIOFilterAdd(s_archive_index, sceFiosIOFilterPsarcDearchiver, &s_dearchiver_context);

	/* ramcache */

	s_ramcache_context.pPath = "app0:data.psarc";
	s_ramcache_context.workBufferSize = sizeof(s_ramcache_work_buffer);
	s_ramcache_context.pWorkBuffer = s_ramcache_work_buffer;
	s_ramcache_context.blockSize = RAMCACHEBLOCKSIZE;
	sceFiosIOFilterAdd(s_archive_index + 1, sceFiosIOFilterCache, &s_ramcache_context);

	SceFiosSize result = sceFiosArchiveGetMountBufferSizeSync(NULL, "app0:data.psarc", NULL);
	s_mount_buffer.length = (SceSize)result;
	s_mount_buffer.pPtr = sceClibMspaceMalloc(mspace, s_mount_buffer.length);
	sceFiosArchiveMountSync(NULL, &s_archive_fh, "app0:data.psarc", "/data", s_mount_buffer, NULL);

	icon_audio = vita2d_load_BMP_file("/data/res/icon_audio.bmp", 1);
	icon_file = vita2d_load_BMP_file("/data/res/icon_file.bmp", 1);
	icon_dir = vita2d_load_BMP_file("/data/res/icon_folder.bmp", 1);
	icon_back = vita2d_load_BMP_file("/data/res/icon_back.bmp", 1);

	btn_forward = vita2d_load_BMP_file("/data/res/btn_playback_forward.bmp", 1);
	btn_pause = vita2d_load_BMP_file("/data/res/btn_playback_pause.bmp", 1);
	btn_play = vita2d_load_BMP_file("/data/res/btn_playback_play.bmp", 1);
	btn_repeat = vita2d_load_BMP_file("/data/res/btn_playback_repeat.bmp", 1);
	btn_rewind = vita2d_load_BMP_file("/data/res/btn_playback_rewind.bmp", 1);
	btn_shuffle = vita2d_load_BMP_file("/data/res/btn_playback_shuffle.bmp", 1);

	toggle_on = vita2d_load_BMP_file("/data/res/toggle_on.bmp", 1);
	toggle_off = vita2d_load_BMP_file("/data/res/toggle_off.bmp", 1);
	radio_on = vita2d_load_BMP_file("/data/res/radio_button_checked.bmp", 1);
	radio_off = vita2d_load_BMP_file("/data/res/radio_button_unchecked.bmp", 1);

	sceFiosArchiveUnmountSync(NULL, s_archive_fh);
	sceClibMspaceFree(mspace, s_mount_buffer.pPtr);
	sceFiosIOFilterRemove(s_archive_index);
	sceFiosIOFilterRemove(s_archive_index + 1);
	sceFiosTerminate();
}

void Textures_Free(void) {
	vita2d_free_texture(radio_off);
	vita2d_free_texture(radio_on);
	vita2d_free_texture(toggle_off);
	vita2d_free_texture(toggle_on);

	vita2d_free_texture(btn_shuffle);
	vita2d_free_texture(btn_rewind);
	vita2d_free_texture(btn_repeat);
	vita2d_free_texture(btn_play);
	vita2d_free_texture(btn_pause);
	vita2d_free_texture(btn_forward);

	vita2d_free_texture(icon_back);
	vita2d_free_texture(icon_dir);
	vita2d_free_texture(icon_file);
	vita2d_free_texture(icon_audio);
}
