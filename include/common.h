#ifndef _ELEVENMPV_COMMON_H_
#define _ELEVENMPV_COMMON_H_

#include <psp2/ctrl.h>
#include <psp2/types.h>
#include <vita2d.h>

/// Checks whether a result code indicates success.
#define R_SUCCEEDED(res)   ((res)>=0)
/// Checks whether a result code indicates failure.
#define R_FAILED(res)      ((res)<0)
/// Returns the level of a result code.

#define MAX_FILES 1024
#define ROOT_PATH "ux0:/"

#define FILES_PER_PAGE 6

extern vita2d_font *font;
extern enum SceCtrlButtons SCE_CTRL_ENTER, SCE_CTRL_CANCEL;
extern SceUInt32 pressed;
extern int position;
extern int file_count;
extern char cwd[512];

#endif
