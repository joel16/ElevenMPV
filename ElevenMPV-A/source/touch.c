#include <systemgesture.h>
#include <touch.h>
#include <kernel.h>
#include <libsysmodule.h>
#include <stdlib.h>

#include "touch_e.h"

static SceSystemGestureTouchRecognizer *touch_recognizers;
static int filelist_drag_accumulator = 0;

int Touch_Init(void) {
	SceTouchPanelInfo front_panel_info;
	sceTouchGetPanelInfo(SCE_TOUCH_PORT_FRONT, &front_panel_info);
	sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);
	sceSysmoduleLoadModule(SCE_SYSMODULE_SYSTEM_GESTURE);
	sceSystemGestureInitializePrimitiveTouchRecognizer(NULL);

	touch_recognizers = (SceSystemGestureTouchRecognizer *)calloc(TOUCH_RECOGNIZERS_NUM, sizeof(SceSystemGestureTouchRecognizer));
	SceSystemGestureRectangle rect;
	rect.x = BTN_PLAY_X * 2;
	rect.y = BTN_MAIN_Y * 2;
	rect.width = BUTTON_WIDTH * 2;
	rect.height = BUTTON_HEIGHT * 2;
	sceSystemGestureCreateTouchRecognizer(&touch_recognizers[TOUCHREC_TAP_PLAY], SCE_SYSTEM_GESTURE_TYPE_TAP, SCE_TOUCH_PORT_FRONT, &rect, NULL);
	rect.x = 20;
	rect.y = BTN_SETTINGS_Y * 2;
	rect.width = 100;
	rect.height = 100;
	sceSystemGestureCreateTouchRecognizer(&touch_recognizers[TOUCHREC_TAP_BACK], SCE_SYSTEM_GESTURE_TYPE_TAP, SCE_TOUCH_PORT_FRONT, &rect, NULL);
	rect.x = (BTN_SETTINGS_X - 25) * 2;
	rect.y = BTN_SETTINGS_Y * 2;
	rect.width = 100;
	rect.height = 100;
	sceSystemGestureCreateTouchRecognizer(&touch_recognizers[TOUCHREC_TAP_SETTINGS], SCE_SYSTEM_GESTURE_TYPE_TAP, SCE_TOUCH_PORT_FRONT, &rect, NULL);
	rect.x = BTN_REW_X * 2;
	rect.y = BTN_MAIN_Y * 2;
	rect.width = BUTTON_WIDTH * 2;
	rect.height = BUTTON_HEIGHT * 2;
	sceSystemGestureCreateTouchRecognizer(&touch_recognizers[TOUCHREC_TAP_REW], SCE_SYSTEM_GESTURE_TYPE_TAP, SCE_TOUCH_PORT_FRONT, &rect, NULL);
	rect.x = BTN_FF_X * 2;
	rect.y = BTN_MAIN_Y * 2;
	rect.width = BUTTON_WIDTH * 2;
	rect.height = BUTTON_HEIGHT * 2;
	sceSystemGestureCreateTouchRecognizer(&touch_recognizers[TOUCHREC_TAP_FF], SCE_SYSTEM_GESTURE_TYPE_TAP, SCE_TOUCH_PORT_FRONT, &rect, NULL);
	rect.x = BTN_SHUFFLE_X * 2;
	rect.y = BTN_SUB_Y * 2;
	rect.width = BUTTON_WIDTH * 2;
	rect.height = BUTTON_HEIGHT * 2;
	sceSystemGestureCreateTouchRecognizer(&touch_recognizers[TOUCHREC_TAP_SHUFFLE], SCE_SYSTEM_GESTURE_TYPE_TAP, SCE_TOUCH_PORT_FRONT, &rect, NULL);
	rect.x = BTN_REPEAT_X * 2;
	rect.y = BTN_SUB_Y * 2;
	rect.width = BUTTON_WIDTH * 2;
	rect.height = BUTTON_HEIGHT * 2;
	sceSystemGestureCreateTouchRecognizer(&touch_recognizers[TOUCHREC_TAP_REPEAT], SCE_SYSTEM_GESTURE_TYPE_TAP, SCE_TOUCH_PORT_FRONT, &rect, NULL);
	rect.x = SEEK_X * 2;
	rect.y = 960;
	rect.width = SEEK_WIDTH * 2;
	rect.height = 50;
	sceSystemGestureCreateTouchRecognizer(&touch_recognizers[TOUCHREC_DRAG_SEEK], SCE_SYSTEM_GESTURE_TYPE_DRAG, SCE_TOUCH_PORT_FRONT, &rect, NULL);
	rect.x = 0;
	rect.y = 224;
	rect.width = 1920;
	rect.height = 864;
	sceSystemGestureCreateTouchRecognizer(&touch_recognizers[TOUCHREC_DRAG_DIRBROWSE], SCE_SYSTEM_GESTURE_TYPE_DRAG, SCE_TOUCH_PORT_FRONT, &rect, NULL);

	return 1;
}

int Touch_GetFileListDragReport(void) {
	SceSystemGestureTouchEvent drag_event;
	SceUInt32 event_num_buffer = 0;
	sceSystemGestureGetTouchEvents(&touch_recognizers[TOUCHREC_DRAG_DIRBROWSE], &drag_event, 1, &event_num_buffer);
	if (event_num_buffer > 0)
		filelist_drag_accumulator += drag_event.property.drag.deltaVector.y;
	if (filelist_drag_accumulator < -80) {
		filelist_drag_accumulator = 0;
		return 1;
	}
	else if (filelist_drag_accumulator > 80) {
		filelist_drag_accumulator = 0;
		return -1;
	}
	else
		return 0;
}

int Touch_GetDragRecStateXPos(int rec_type) {
	SceSystemGestureTouchEvent drag_event;
	SceUInt32 event_num_buffer = 0;
	sceSystemGestureGetTouchEvents(&touch_recognizers[rec_type], &drag_event, 1, &event_num_buffer);
	if (event_num_buffer > 0) {
		return (drag_event.property.drag.currentPosition.x / 2);
	}
	else
		return -1;
}

int Touch_GetTapRecState(int rec_type) {
	SceSystemGestureTouchEvent tap_event;
	SceUInt32 event_num_buffer = 0;
	sceSystemGestureGetTouchEvents(&touch_recognizers[rec_type], &tap_event, 1, &event_num_buffer);
	if (event_num_buffer > 0)
		return 1;
	else
		return 0;
}

void Touch_ChangeRecRectangle(int rec_type, SceInt16 new_x, SceInt16 new_width, SceInt16 new_y, SceInt16 new_height) {
	SceSystemGestureRectangle rect;
	rect.x = new_x;
	rect.y = new_y;
	rect.width = new_width;
	rect.height = new_height;
	sceSystemGestureUpdateTouchRecognizerRectangle(&touch_recognizers[rec_type], &rect);
}
	
void Touch_Update(void) {
	SceTouchData tdf;
	sceTouchPeek(SCE_TOUCH_PORT_FRONT, &tdf, 1);
	sceSystemGestureUpdatePrimitiveTouchRecognizer(&tdf, NULL);
	for (int i = 0; i < TOUCH_RECOGNIZERS_NUM; i++)
		sceSystemGestureUpdateTouchRecognizer(&touch_recognizers[i]);
}
