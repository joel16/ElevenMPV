#ifndef _ELEVENMPV_DIRBROWSE_H_
#define _ELEVENMPV_DIRBROWSE_H_

typedef struct File {
	struct File *next; // Next item
	SceBool is_dir;        // Folder flag
	char name[256];    // File name
	char ext[5];       // File extension
} File;

extern File *files;

void Dirbrowse_SaveLastDirectory(void);
int Dirbrowse_PopulateFiles(SceBool clear);
void Dirbrowse_DisplayFiles(void);
File *Dirbrowse_GetFileIndex(int index);
void Dirbrowse_OpenFile(void);
int Dirbrowse_Navigate(SceBool parent);

#endif
