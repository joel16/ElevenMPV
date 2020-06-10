#ifndef _ELEVENMPV_MOTION_H_
#define _ELEVENMPV_MOTION_H_

typedef enum MotionCommands {
	MOTION_NEXT,
	MOTION_PREVIOUS,
	MOTION_STOP
} MotionCommands;

void Motion_SetState(SceBool state);
int Motion_GetCommand(void);
void Motion_SetReleaseTimer(SceUInt32 timer);
void Motion_SetAngleThreshold(SceUInt32 threshold);

#endif
