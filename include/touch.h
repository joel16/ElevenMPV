/*
		
		Taken from VitaComix by Arkanite.
		A manga/comic book reader for the Playstation Vita
		
		File: touch.c
		
*/
#ifndef _ELEVENMPV_TOUCH_H_
#define _ELEVENMPV_TOUCH_H_

#define TOUCH_RECOGNIZERS_NUM 9

#define BUTTON_WIDTH  68
#define BUTTON_HEIGHT 68

#define BTN_SETTINGS_X 900

#define BTN_MAIN_Y (124 + ((400 - BUTTON_HEIGHT) / 2))
#define BTN_SUB_Y (BTN_MAIN_Y + 100)

#define BTN_PLAY_X (480 - BUTTON_WIDTH / 2)
#define BTN_REW_X (BTN_PLAY_X - 136)
#define BTN_FF_X (BTN_PLAY_X + 136)
#define BTN_SHUFFLE_X (BTN_PLAY_X - 90)
#define BTN_REPEAT_X (BTN_PLAY_X + 90)

#define SEEK_X 50
#define SEEK_WIDTH 860
#define SEEK_WIDTH_FLOAT 860.0

typedef enum SceSystemGestureTouchRecognizerTypes {
	TOUCHREC_TAP_PLAY,
	TOUCHREC_TAP_REW,
	TOUCHREC_TAP_FF,
	TOUCHREC_TAP_BACK,
	TOUCHREC_TAP_SETTINGS,
	TOUCHREC_TAP_SHUFFLE,
	TOUCHREC_TAP_REPEAT,
	TOUCHREC_DRAG_SEEK,
	TOUCHREC_DRAG_DIRBROWSE
} SceSystemGestureTouchRecognizerTypes;

int Touch_Init(void);
void Touch_Shutdown(void);
int Touch_GetFileListDragReport(void);
int Touch_GetDragRecStateXPos(int rec_type);
int Touch_GetTapRecState(int rec_type);
void Touch_ChangeRecRectangle(int rec_type, SceInt16 new_x, SceInt16 new_width, SceInt16 new_y, SceInt16 new_height);

void Touch_Update(void);

#endif
