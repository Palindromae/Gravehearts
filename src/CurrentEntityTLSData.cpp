#include "CurrentEntityTLSData.h"

namespace TLS {
	InterpolatedFrameData* InterpolatedFrame = new InterpolatedFrameData();
	EntityFrameData* PreviousInterpolationFrame = new EntityFrameData();
	
	/// Actives -- VOLITILE LENGTH
	int NumberOfActiveEntities = 0;
	int* ActiveEntities = nullptr;

}
