#include <psp2/kernel/clib.h>
#include <psp2/io/dirent.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/libc.h>
#include <psp2/gxm.h>

#include "common.h"
#include "config.h"
#include "dirbrowse.h"
#include "fs.h"
#include "menu_audioplayer.h"
#include "textures.h"
#include "utils.h"

File *files = NULL;

static int xdelta_topbar = 0, xdelta_file = 0;
static SceBool flip_topbar = SCE_FALSE, flip_file = SCE_FALSE;
static int delay_topbar = 0, delay_file = 0;

static void Dirbrowse_RecursiveFree(File *node) {
	if (node == NULL) // End of list
		return;

	Dirbrowse_RecursiveFree(node->next); // Nest further

	sceLibcFree(node); // Free memory
}

static void Dirbrowse_SaveLastDirectory(void) {
	int len = sceClibStrnlen(cwd, 256);
	Utils_WriteSafeMem((void*)&len, 4, 0);
	Utils_WriteSafeMem((void*)cwd, len, 4);
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
			return sceLibcStrcasecmp(entryA->d_name, entryB->d_name);
		else if (config.sort == 1) // Sort alphabetically (descending - Z to A)
			return sceLibcStrcasecmp(entryB->d_name, entryA->d_name);
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
		SceIoDirent *entries = (SceIoDirent *)sceLibcCalloc(MAX_FILES, sizeof(SceIoDirent));

		while (sceIoDread(dir, &entries[entryCount]) > 0)
			entryCount++;

		sceIoDclose(dir);
		sceLibcQsort(entries, entryCount, sizeof(SceIoDirent), cmpstringp);

		for (int i = -1; i < entryCount; i++) {
			// Allocate Memory
			File *item = (File *)sceLibcMalloc(sizeof(File));

			sceClibMemset(item, 0, sizeof(File));

			if ((sceClibStrcmp(cwd, root_path)) && (i == -1) && (!parent_dir_set)) {
				sceLibcStrcpy(item->name, "..");
				item->is_dir = SCE_TRUE;
				parent_dir_set = SCE_TRUE;
				file_count++;
			}
			else {
				if ((i == -1) && (!(sceClibStrcmp(cwd, root_path))))
					continue;

				item->is_dir = SCE_S_ISDIR(entries[i].d_stat.st_mode);
				// Copy File Name
				sceLibcStrcpy(item->name, entries[i].d_name);
				sceLibcStrcpy(item->ext, FS_GetFileExt(item->name));
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

		sceLibcFree(entries);
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

	int width = vita2d_pvf_text_width(font, 1, cwd);
	int delta = 0;

	if (width > 750) {
		delta = width - 750;
		switch (flip_topbar) {
		case SCE_FALSE:
			if (xdelta_topbar > -delta)
				xdelta_topbar--;
			else {
				if (delay_topbar < FLIP_DELAY)
					delay_topbar++;
				else {
					delay_topbar = 0;
					flip_topbar = SCE_TRUE;
				}
			}
			break;
		case SCE_TRUE:
			if (xdelta_topbar < 0)
				xdelta_topbar++;
			else {
				if (delay_topbar < FLIP_DELAY)
					delay_topbar++;
				else {
					delay_topbar = 0;
					flip_topbar = SCE_FALSE;
				}
			}
			break;
		}
	}
	else
		xdelta_topbar = 0;

	vita2d_set_region_clip(SCE_GXM_REGION_CLIP_OUTSIDE, 102, 0, 850, 544);
	vita2d_pvf_draw_text(font, 102 + xdelta_topbar, 40 + ((72 - vita2d_pvf_text_height(font, 1, cwd)) / 2) + 20, RGBA8(255, 255, 255, 255), 1, cwd);
	vita2d_set_region_clip(SCE_GXM_REGION_CLIP_OUTSIDE, 0, 0, 960, 544);

	if (!(!sceClibStrcmp(cwd, root_path)))
		vita2d_draw_texture(icon_back, BTN_BACK_X, BTN_TOPBAR_Y);

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
				|| (!sceClibStrncasecmp(file->ext, "s3m", 4))|| (!sceClibStrncasecmp(file->ext, "wav", 4)) || (!sceClibStrncasecmp(file->ext, "xm", 4))
				|| (!sceClibStrncasecmp(file->ext, "at9", 4)) || (!sceClibStrncasecmp(file->ext, "m4a", 4)) || (!sceClibStrncasecmp(file->ext, "aac", 4)))
				vita2d_draw_texture(icon_audio, 15, 117 + (72 * printed));
			else
				vita2d_draw_texture(icon_file, 15, 117 + (72 * printed));

			if (sceClibStrncmp(file->name, "..", 2) == 0)
				vita2d_pvf_draw_text(font, 102, 120 + (72 / 2) + (72 * printed), RGBA8(51, 51, 51, 255), 1, "Parent folder");
			else if (i == position) {

				width = vita2d_pvf_text_width(font, 1, file->name);

				if (width > 750) {
					delta = width - 750;
					switch (flip_file) {
					case SCE_FALSE:
						if (xdelta_file > -delta)
							xdelta_file--;
						else {
							if (delay_file < FLIP_DELAY)
								delay_file++;
							else {
								delay_file = 0;
								flip_file = SCE_TRUE;
							}
						}
						break;
					case SCE_TRUE:
						if (xdelta_file < 0)
							xdelta_file++;
						else {
							if (delay_file < FLIP_DELAY)
								delay_file++;
							else {
								delay_file = 0;
								flip_file = SCE_FALSE;
							}
						}
						break;
					}
				}
				else
					xdelta_file = 0;

				vita2d_set_region_clip(SCE_GXM_REGION_CLIP_OUTSIDE, 102, 0, 960, 544);
				vita2d_pvf_draw_text(font, 102 + xdelta_file, 120 + (72 / 2) + (72 * printed), RGBA8(51, 51, 51, 255), 1, file->name);
				vita2d_set_region_clip(SCE_GXM_REGION_CLIP_OUTSIDE, 0, 0, 960, 544);
			}
			else
				vita2d_pvf_draw_text(font, 102, 120 + (72 / 2) + (72 * printed), RGBA8(51, 51, 51, 255), 1, file->name);

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

	sceLibcStrcpy(path, cwd);
	sceLibcStrcpy(path + sceLibcStrlen(path), file->name);

	if (file->is_dir) {
		// Attempt to navigate to target
		if (R_SUCCEEDED(Dirbrowse_Navigate(SCE_FALSE))) {
			Dirbrowse_SaveLastDirectory();
			Dirbrowse_PopulateFiles(SCE_TRUE);
		}
	}
	else if ((!sceClibStrncasecmp(file->ext, "flac", 4)) || (!sceClibStrncasecmp(file->ext, "it", 4)) || (!sceClibStrncasecmp(file->ext, "mod", 4))
		|| (!sceClibStrncasecmp(file->ext, "mp3", 4)) || (!sceClibStrncasecmp(file->ext, "ogg", 4)) || (!sceClibStrncasecmp(file->ext, "opus", 4))
		|| (!sceClibStrncasecmp(file->ext, "s3m", 4)) || (!sceClibStrncasecmp(file->ext, "wav", 4)) || (!sceClibStrncasecmp(file->ext, "xm", 4))
		|| (!sceClibStrncasecmp(file->ext, "at9", 4)) || (!sceClibStrncasecmp(file->ext, "m4a", 4)) || (!sceClibStrncasecmp(file->ext, "aac", 4))) {
		Menu_PlayAudio(path);
	}
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
		int i = sceLibcStrlen(cwd) - 2; 
		for(; i >= 0; i--) {
			// Slash discovered
			if (cwd[i] == '/') {
				slash = cwd + i + 1; // Save pointer
				break; // Stop search
			}
		}

		//slash = sceClibStrrchr(cwd, '/');

		slash[0] = 0; // Terminate working directory
	}
	// Normal folder
	else {
		if (file->is_dir) {
			// Append folder to working directory
			sceLibcStrcpy(cwd + sceLibcStrlen(cwd), file->name);
			cwd[sceLibcStrlen(cwd) + 1] = 0;
			cwd[sceLibcStrlen(cwd)] = '/';
		}
	}

	Dirbrowse_SaveLastDirectory();
	return 0; // Return success
}
