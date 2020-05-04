#include <psp2/kernel/clib.h>
#include <psp2/io/dirent.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>

#include "common.h"
#include "fs.h"

SceBool FS_FileExists(const char *path) {
	SceUID file = 0;
	
	if (R_SUCCEEDED(file = sceIoOpen(path, SCE_O_RDONLY, 0777))) {
		sceIoClose(file);
		return SCE_TRUE;
	}
	
	return SCE_FALSE;
}

SceBool FS_DirExists(const char *path) {
	SceUID dir = 0;
	
	if (R_SUCCEEDED(dir = sceIoDopen(path))) {
		sceIoDclose(dir);
		return SCE_TRUE;
	}
	
	return SCE_FALSE;
}

const char *FS_GetFileExt(const char *filename) {
	const char *dot = sceClibStrrchr(filename, '.');
	
	if (!dot || dot == filename)
		return "";
	
	return dot + 1;
}

int FS_GetFileSize(const char *path, SceOff *size) {
	SceIoStat stat;
	int ret = 0;

	if (R_FAILED(ret = sceIoGetstat(path, &stat)))
		return ret;

	*size = stat.st_size;

	return 0;
}

int FS_ReadFile(const char *path, void *buf, int size) {
	SceUID file = 0;

	if (R_SUCCEEDED(file = sceIoOpen(path, SCE_O_RDONLY, 0))) {
		int bytes_read = sceIoRead(file, buf, size);
		sceIoClose(file);
		return bytes_read;
	}
	
	return file;
}

int FS_WriteFile(char *path, void *buf, int size) {
	SceUID file = 0;
	
	if (R_SUCCEEDED(file = sceIoOpen(path, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777))) {
		int bytes_written = sceIoWrite(file, buf, size);
		sceIoClose(file);
		return bytes_written;
	}

	return file;
}
