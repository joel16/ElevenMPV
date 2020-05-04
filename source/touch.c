/*
		
		Taken from VitaComix by Arkanite.
		A manga/comic book reader for the Playstation Vita
		
		File: touch.c
		
*/
#include <psp2/touch.h>
#include <psp2/kernel/clib.h>

#include "touch.h"

#define lerp(value, from_max, to_max) ((((value * 10) * (to_max * 10)) / (from_max * 10)) / 10)

static SceTouchData touch;

extern void* mspace;

typedef struct {
	int posX;
	int posY;
	int held;
	int pressed;
	int pressedPrev;
	int released;
	int releasedPrev;
} touchStateData;

touchStateData *touchState;

void Touch_Reset(void) {
	touchState->posX = 0;
	touchState->posY = 0;
	touchState->held = 0;
	touchState->pressed = 0;
	touchState->pressedPrev = 0;
	touchState->released = 0;
	touchState->releasedPrev = 0;
}

int Touch_Init(void) {
	sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, 1);
	
	touchState = sceClibMspaceMalloc(mspace, sizeof(touchStateData));
	Touch_Reset();
	
	return 1;
}

void Touch_Shutdown(void) {
	sceClibMspaceFree(mspace, touchState);
}

int Touch_GetX(void) {
	return touchState->posX;
}

int Touch_GetY(void) {
	return touchState->posY;
}

int Touch_CheckPressed(void) {
	return touchState->pressed;
}

int Touch_CheckReleased(void) {
	return touchState->released;
}

int Touch_CheckHeld(void) {
	return touchState->held;
}
	
void Touch_Update(void) {
	sceTouchPeek(SCE_TOUCH_PORT_FRONT, &touch, 1);
	
	if (touch.reportNum > 0) {
		touchState->held = 1;
		touchState->posX = (lerp(touch.report[0].x, 1919, 960));
		touchState->posY = (lerp(touch.report[0].y, 1087, 544));
		touchState->released = 0;
		touchState->releasedPrev = 0;
		if (touchState->pressedPrev == 0) {
			touchState->pressedPrev = 1;
			touchState->pressed = 1;
		}
		else 
			touchState->pressed = 0;
	}
	else {
		touchState->held = 0;
		touchState->pressed = 0;
		touchState->pressedPrev = 0;
		if (touchState->releasedPrev == 0) {
			touchState->releasedPrev = 1;
			touchState->released = 1;
		}
		else 
			touchState->released = 0;
	}
}
