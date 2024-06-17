#pragma once
#include "InterpolatedFrameData.h"

namespace TLS {
	extern InterpolatedFrameData* InterpolatedFrame;
	extern EntityFrameData* PreviousInterpolationFrame;
	/// Activities -- VOLITILE LENGTH
	extern int NumberOfActiveEntities;
	extern int* ActiveEntities;
}
