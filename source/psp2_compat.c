#include <psp2/kernel/clib.h>
#include <psp2/kernel/modulemgr.h>
#include <stdio.h>
#include <stdlib.h>

#include "psp2_compat.h"

char* varHelperSceLibc_ctype(void);
int* _sceLibcErrnoLoc(void);

char* __ctype_ptr__;
struct _reent *_impure_ptr;

void psp2CompatInit(void)
{
	SceUID modid;
	modid = sceKernelLoadStartModule("app0:module/user/VarHelper.suprx", 0, NULL, 0, NULL, NULL);
	__ctype_ptr__ = varHelperSceLibc_ctype();
	sceKernelStopUnloadModule(modid, 0, NULL, 0, NULL, NULL);
	_impure_ptr = malloc(sizeof(struct _reent));
}

//Fake stat() for libxmp
int stat(const char *pathname, void *statbuf)
{
	return 1;
}

int fseeko(FILE *stream, off_t offset, int whence)
{
	return fseek(stream, offset, whence);
}

off_t ftello(FILE *stream)
{
	return ftell(stream);
}

int* __errno(void)
{
	return _sceLibcErrnoLoc();
}