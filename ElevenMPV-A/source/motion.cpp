#include <kernel.h>
#include <motion.h>
#include <paf.h>

#include "motion_e.h"

static SceMotionState s_motionStateNew, s_motionStateOld;
static SceUInt32 s_releaseTimer = 0, s_motionTimer = 0;
static SceBool s_motionBusy = SCE_FALSE;
static SceInt32 s_motionCommandX = -2, s_motionCommandZ = -2;
static SceUInt32 s_cooldownCounter = 0;

SceVoid motion::Motion::SetState(SceBool state)
{
	if (state) {
		sceMotionStartSampling();
		sceMotionSetTiltCorrection(SCE_FALSE);
		//sceMotionMagnetometerOff();

		sce_paf_memset(&s_motionStateNew, 0, sizeof(SceMotionState));
		sce_paf_memset(&s_motionStateOld, 0, sizeof(SceMotionState));
	}
	else {
		sceMotionStopSampling();
	}
}

SceInt32 motion::Motion::GetCommand()
{
	SceInt32 ret = -1;

	if (s_cooldownCounter != 0)
		s_cooldownCounter--;
	else {
		sceMotionGetState(&s_motionStateNew);

		if (!s_motionBusy && s_motionStateNew.basicOrientation.x != 0) {
			s_motionBusy = SCE_TRUE;
			s_motionTimer = sceKernelGetProcessTimeLow();
			s_motionCommandX = (SceInt32)s_motionStateNew.basicOrientation.x;
			s_motionCommandZ = -2;
		}
		else if (!s_motionBusy && s_motionStateNew.basicOrientation.z == 1) {
			s_motionBusy = SCE_TRUE;
			s_motionTimer = sceKernelGetProcessTimeLow();
			s_motionCommandZ = (SceInt32)s_motionStateNew.basicOrientation.z;
			s_motionCommandX = -2;
		}

		if (s_motionBusy) {

			if ((sceKernelGetProcessTimeLow() - s_motionTimer) > s_releaseTimer * 1000000) {
				s_motionBusy = SCE_FALSE;
			}

			if (s_motionStateNew.basicOrientation.x == 0 && s_motionCommandZ == -2) {
				s_motionBusy = SCE_FALSE;
				s_cooldownCounter = COOLDOWN_VALUE;
				switch (s_motionCommandX) {
				case -1:
					return MOTION_NEXT;
					break;
				case 1:
					return MOTION_PREVIOUS;
					break;
				}
			}
			else if (s_motionStateNew.basicOrientation.z == 0 && s_motionCommandX == -2) {
				s_motionBusy = SCE_FALSE;
				s_cooldownCounter = COOLDOWN_VALUE;
				return MOTION_STOP;
			}
		}
	}

	return ret;
}

SceVoid motion::Motion::SetReleaseTimer(SceUInt32 timer)
{
	s_releaseTimer = timer;
}

SceVoid motion::Motion::SetAngleThreshold(SceUInt32 threshold)
{
	sceMotionSetAngleThreshold((SceFloat32)threshold);
}