#ifndef _ELEVENMPV_FS_H_
#define _ELEVENMPV_FS_H_

SceBool FS_FileExists(const char *path);
SceBool FS_DirExists(const char *path);
const char *FS_GetFileExt(const char *filename);
int FS_GetFileSize(const char *path, SceOff *size);
int FS_ReadFile(const char *path, void *buf, int size);
int FS_WriteFile(char *path, void *buf, int size);

#endif
