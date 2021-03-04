#include <kernel.h>
#include <motion.h>

#include "motion_e.h"

static SceMotionState motion_state_new, motion_state_old;
static SceUInt32 release_timer = 0, motion_timer = 0;
static SceBool motion_busy = SCE_FALSE;
static int motion_command_x = -1, motion_command_z = -1;

void Motion_SetState(SceBool state) {
	if (state) {
		sceMotionStartSampling();
		sceMotionSetTiltCorrection(0);
	}
	else {
		sceMotionStopSampling();
	}
}

int Motion_GetCommand(void) {
	int ret = 0;
	sceMotionGetState(&motion_state_new);
	if (motion_state_new.basicOrientation.x != 0 && motion_state_old.basicOrientation.x == 0) {
		motion_timer = sceKernelGetProcessTimeLow();
		motion_busy = SCE_TRUE;
		motion_command_x = -1;
		motion_command_z = -1;
		ret = -1;
	}
	if (motion_state_new.basicOrientation.z == -1 && motion_state_old.basicOrientation.z == 0) {
		motion_timer = sceKernelGetProcessTimeLow();
		motion_busy = SCE_TRUE;
		motion_command_x = -1;
		motion_command_z = -1;
		ret = -1;
	}
	if (motion_busy) {
		switch ((int)motion_state_new.basicOrientation.x) {
		case -1:
			motion_command_x = MOTION_NEXT;
			break;
		case 1:
			motion_command_x = MOTION_PREVIOUS;
			break;
		case 0:
			if (motion_command_x != -1) {
				ret = motion_command_x;
				motion_busy = SCE_FALSE;
			}
			break;
		}
		switch ((int)motion_state_new.basicOrientation.z) {
		case -1:
			motion_command_z = MOTION_STOP;
			break;
		case 0:
			if (motion_command_z != -1) {
				ret = motion_command_z;
				motion_busy = SCE_FALSE;
			}
			break;
		}
		if ((sceKernelGetProcessTimeLow() - motion_timer) > release_timer) {
			motion_busy = SCE_FALSE;
		}
	}
	sceClibMemcpy(&motion_state_old.basicOrientation, &motion_state_new.basicOrientation, sizeof(SceFVector3));
	return ret;
}

void Motion_SetReleaseTimer(SceUInt32 timer) {
	release_timer = timer;
}

void Motion_SetAngleThreshold(SceUInt32 threshold) {
	sceMotionSetAngleThreshold(threshold);
}