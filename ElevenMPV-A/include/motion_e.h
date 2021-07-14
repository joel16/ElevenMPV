#ifndef _ELEVENMPV_MOTION_H_
#define _ELEVENMPV_MOTION_H_

#define COOLDOWN_VALUE 50

namespace motion {

	class Motion
	{
	public:

		enum Command 
		{
			MOTION_NEXT,
			MOTION_PREVIOUS,
			MOTION_STOP
		};

		static SceVoid SetState(SceBool enable);

		static SceInt32 GetCommand();

		static SceVoid SetReleaseTimer(SceUInt32 timer);

		static SceVoid SetAngleThreshold(SceUInt32 threshold);
	};
}

#endif
