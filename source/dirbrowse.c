#include <psp2/kernel/clib.h>
#include <psp2/io/dirent.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "config.h"
#include "dirbrowse.h"
#include "fs.h"
#include "menu_audioplayer.h"
#include "textures.h"
#include "utils.h"

File *files = NULL;

extern void* mspace;
void* sceClibMspaceCalloc(void* space, size_t num, size_t size);

static void Dirbrowse_RecursiveFree(File *node) {
	if (node == NULL) // End of list
		return;
	
	Dirbrowse_RecursiveFree(node->next); // Nest further
	sceClibMspaceFree(mspace, node); // Free memory
}

static void Dirbrowse_SaveLastDirectory(void) {
	char *buf = sceClibMspaceMalloc(mspace, 256);
	int len = sceClibSnprintf(buf, 256, "%s\n", cwd);
	FS_WriteFile("savedata0:lastdir.txt", buf, len);
	sceClibMspaceFree(mspace, buf);
}

static int cmpstringp(const void *p1, const void *p2) {
	SceIoDirent *entryA = (SceIoDirent *)p1;
	SceIoDirent *entryB = (SceIoDirent *)p2;

	if ((SCE_S_ISDIR(entryA->d_stat.st_mode)) && !(SCE_S_ISDIR(entryB->d_stat.st_mode)))
		return -1;
	else if (!(SCE_S_ISDIR(entryA->d_stat.st_mode)) && (SCE_S_ISDIR(entryB->d_stat.st_mode)))
		return 1;
	else {
		if (config.sort == 0) // Sort alphabetically (ascending - A to Z)
			return strcasecmp(entryA->d_name, entryB->d_name);
		else if (config.sort == 1) // Sort alphabetically (descending - Z to A)
			return strcasecmp(entryB->d_name, entryA->d_name);
		else if (config.sort == 2) // Sort by file size (largest first)
			return entryA->d_stat.st_size > entryB->d_stat.st_size ? -1 : entryA->d_stat.st_size < entryB->d_stat.st_size ? 1 : 0;
		else if (config.sort == 3) // Sort by file size (smallest first)
			return entryB->d_stat.st_size > entryA->d_stat.st_size ? -1 : entryB->d_stat.st_size < entryA->d_stat.st_size ? 1 : 0;
	}

	return 0;
}

int Dirbrowse_PopulateFiles(SceBool refresh) {
	SceUID dir = 0;
	Dirbrowse_RecursiveFree(files);
	files = NULL;
	file_count = 0;

	SceBool parent_dir_set = SCE_FALSE;

	if (R_SUCCEEDED(dir = sceIoDopen(cwd))) {
		int entryCount = 0;
		SceIoDirent *entries = (SceIoDirent *)sceClibMspaceCalloc(mspace, MAX_FILES, sizeof(SceIoDirent));

		while (sceIoDread(dir, &entries[entryCount]) > 0)
			entryCount++;

		sceIoDclose(dir);
		qsort(entries, entryCount, sizeof(SceIoDirent), cmpstringp);

		for (int i = -1; i < entryCount; i++) {
			// Allocate Memory
			File *item = (File *)sceClibMspaceMalloc(mspace, sizeof(File));
			sceClibMemset(item, 0, sizeof(File));

			if ((sceClibStrcmp(cwd, root_path)) && (i == -1) && (!parent_dir_set)) {
				strcpy(item->name, "..");
				item->is_dir = SCE_TRUE;
				parent_dir_set = SCE_TRUE;
				file_count++;
			}
			else {
				if ((i == -1) && (!(sceClibStrcmp(cwd, root_path))))
					continue;

				item->is_dir = SCE_S_ISDIR(entries[i].d_stat.st_mode);

				// Copy File Name
				strcpy(item->name, entries[i].d_name);
				strcpy(item->ext, FS_GetFileExt(item->name));
				file_count++;
			}

			// New List
			if (files == NULL) 
				files = item;

			// Existing List
			else {
				File *list = files;
					
				while(list->next != NULL) 
					list = list->next;

				list->next = item;
			}
		}

		sceClibMspaceFree(mspace, entries);
	}
	else
		return dir;

	if (!refresh) {
		if (position >= file_count) 
			position = file_count - 1; // Keep index
	}
	else 
		position = 0; // Refresh position

	return 0;
}

void Dirbrowse_DisplayFiles(void) {
	vita2d_font_draw_text(font, 102, 40 + ((72 - vita2d_font_text_height(font, 25, cwd)) / 2) + 20, RGBA8(255, 255, 255, 255), 25, cwd);

	if (!(!sceClibStrcmp(cwd, root_path)))
		vita2d_draw_texture(icon_back, 25, 54);

	int i = 0, printed = 0;
	File *file = files; // Draw file list

	for(; file != NULL; file = file->next) {
		if (printed == FILES_PER_PAGE) // Limit the files per page
			break;

		if (position < FILES_PER_PAGE || i > (position - FILES_PER_PAGE)) {
			if (i == position)
				vita2d_draw_rectangle(0, 112 + (72 * printed), 960, 72, RGBA8(230, 230, 230, 255));

			if (file->is_dir)
				vita2d_draw_texture(icon_dir, 15, 117 + (72 * printed));
			else if ((!sceClibStrncasecmp(file->ext, "flac", 4)) || (!sceClibStrncasecmp(file->ext, "it", 4)) || (!sceClibStrncasecmp(file->ext, "mod", 4))
				|| (!sceClibStrncasecmp(file->ext, "mp3", 4)) || (!sceClibStrncasecmp(file->ext, "ogg", 4)) || (!sceClibStrncasecmp(file->ext, "opus", 4))
				|| (!sceClibStrncasecmp(file->ext, "s3m", 4))|| (!sceClibStrncasecmp(file->ext, "wav", 4)) || (!sceClibStrncasecmp(file->ext, "xm", 4)))
				vita2d_draw_texture(icon_audio, 15, 117 + (72 * printed));
			else
				vita2d_draw_texture(icon_file, 15, 117 + (72 * printed));

			if (sceClibStrncmp(file->name, "..", 2) == 0)
				vita2d_font_draw_text(font, 102, 120 + (72 / 2) + (72 * printed), RGBA8(51, 51, 51, 255), 25, "Parent folder");
			else 
				vita2d_font_draw_text(font, 102, 120 + (72 / 2) + (72 * printed), RGBA8(51, 51, 51, 255), 25, file->name);

			printed++; // Increase printed counter
		}

		i++; // Increase counter
	}
}

File *Dirbrowse_GetFileIndex(int index) {
	int i = 0;
	File *file = files; // Find file Item
	
	for(; file != NULL && i != index; file = file->next)
		i++;

	return file; // Return file
}

void Dirbrowse_OpenFile(void) {
	char path[512];
	File *file = Dirbrowse_GetFileIndex(position);

	if (file == NULL)
		return;

	strcpy(path, cwd);
	strcpy(path + strlen(path), file->name);

	if (file->is_dir) {
		// Attempt to navigate to target
		if (R_SUCCEEDED(Dirbrowse_Navigate(SCE_FALSE))) {
			Dirbrowse_SaveLastDirectory();
			Dirbrowse_PopulateFiles(SCE_TRUE);
		}
	}
	else if ((!sceClibStrncasecmp(file->ext, "flac", 4)) || (!sceClibStrncasecmp(file->ext, "it", 4)) || (!sceClibStrncasecmp(file->ext, "mod", 4))
		|| (!sceClibStrncasecmp(file->ext, "mp3", 4)) || (!sceClibStrncasecmp(file->ext, "ogg", 4)) || (!sceClibStrncasecmp(file->ext, "opus", 4))
		|| (!sceClibStrncasecmp(file->ext, "s3m", 4))|| (!sceClibStrncasecmp(file->ext, "wav", 4)) || (!sceClibStrncasecmp(file->ext, "xm", 4)))
		Menu_PlayAudio(path);
}

// Navigate to Folder
int Dirbrowse_Navigate(SceBool parent) {
	File *file = Dirbrowse_GetFileIndex(position); // Get index
	
	if (file == NULL)
		return -1;

	// Special case ".."
	if ((parent) || (!sceClibStrncmp(file->name, "..", 2))) {
		char *slash = NULL;

		// Find last '/' in working directory
		int i = strlen(cwd) - 2; for(; i >= 0; i--) {
			// Slash discovered
			if (cwd[i] == '/') {
				slash = cwd + i + 1; // Save pointer
				break; // Stop search
			}
		}

		slash[0] = 0; // Terminate working directory
	}

	// Normal folder
	else {
		if (file->is_dir) {
			// Append folder to working directory
			strcpy(cwd + strlen(cwd), file->name);
			cwd[strlen(cwd) + 1] = 0;
			cwd[strlen(cwd)] = '/';
		}
	}

	Dirbrowse_SaveLastDirectory();

	return 0; // Return success
}
