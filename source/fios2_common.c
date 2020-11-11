#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/modulemgr.h> 
#include <psp2/kernel/clib.h>
#include <psp2/fios2.h>

// FIOS2 stuff
//--------------------------------------------------------------------------------------
#define MAX_PATH_LENGTH 256

static SceInt64 s_op_storage[SCE_FIOS_OP_STORAGE_SIZE(64, MAX_PATH_LENGTH) / sizeof(SceInt64) + 1];
static SceInt64 s_chunk_storage[SCE_FIOS_CHUNK_STORAGE_SIZE(1024) / sizeof(SceInt64) + 1];
static SceInt64 s_fh_storage[SCE_FIOS_FH_STORAGE_SIZE(32, MAX_PATH_LENGTH) / sizeof(SceInt64) + 1];
static SceInt64 s_dh_storage[SCE_FIOS_DH_STORAGE_SIZE(32, MAX_PATH_LENGTH) / sizeof(SceInt64) + 1];

/*SceFiosRamCacheContext s_ramcache_context = SCE_FIOS_RAM_CACHE_CONTEXT_INITIALIZER;
static SceByte s_ramcache_work_buffer[10 * (64 * 1024)] __attribute__((aligned(8)));*/

int fios2Init(void) {
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

	params.threadAffinity[0] = SCE_KERNEL_CPU_MASK_USER_3;
	params.threadAffinity[1] = SCE_KERNEL_CPU_MASK_USER_3;
	params.threadAffinity[2] = SCE_KERNEL_CPU_MASK_USER_3;

	return sceFiosInitialize(&params);
}

int fios2CommonOpDeleteCB(void *pContext, SceFiosOp op, SceFiosOpEvent event, int err) {

	if (event == SCE_FIOS_OPEVENT_COMPLETE) {
		sceFiosOpDelete(op);
	}

	return 0;
}