#include <dolcesdkkern.h>
#include <taihen.h>
#include <psp2kern/kernel/modulemgr.h> 

static SceUID g_hooks[1];

static int first = 0;

static tai_hook_ref_t ref_hook0;
int ksceSblACMgrIsNonGameProgram_patched(void)
{
	if (!first) {
		first = 1;
		return 1;
	}
	else
		return 0;
}

void _start() __attribute__((weak, alias("module_start")));
int module_start(SceSize argc, const void *args)
{
	g_hooks[0] = taiHookFunctionExportForKernel(KERNEL_PID, &ref_hook0,
		"SceSblACMgr", 0x9AD8E213, 0x6C5AB07F, ksceSblACMgrIsNonGameProgram_patched);

	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args)
{
	if (g_hooks[0] >= 0) taiHookReleaseForKernel(g_hooks[0], ref_hook0);
	return SCE_KERNEL_STOP_SUCCESS;
}