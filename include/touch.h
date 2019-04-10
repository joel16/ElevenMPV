/*
		
		Taken from VitaComix by Arkanite.
		A manga/comic book reader for the Playstation Vita
		
		File: touch.c
		
*/
#ifndef _ELEVENMPV_TOUCH_H_
#define _ELEVENMPV_TOUCH_H_

#define Touch_Position(x1, y1, x2, y2)  ((Touch_CheckPressed()) && (Touch_GetX() >= (x1) && Touch_GetX() <= (x2) && Touch_GetY() >= (y1) && Touch_GetY() <= (y2)))

void Touch_Reset(void);
int Touch_Init(void);
void Touch_Shutdown(void);
int Touch_GetX(void);
int Touch_GetY(void);
int Touch_CheckPressed(void);
int Touch_CheckReleased(void);
int Touch_CheckHeld(void);
void Touch_Update(void);

#endif
