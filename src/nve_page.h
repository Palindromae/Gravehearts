#pragma once
#include <stdint.h>
namespace nve {

	#define nvePageID int

	struct nvePage {
	private:
	public:
		unsigned long long deviceMemloc;
		unsigned long size;

		uint32_t IsReservior;//index onto the resivours, LSB = TINY(0) VS SMALL(1) VS FAST(2)

		nvePage(long int binding, long int size) : deviceMemloc(binding), size(size), IsReservior(0)
		{
		}

		nvePage(long int binding, long int size, uint32_t IsReservior): deviceMemloc(binding), size(size), IsReservior(IsReservior) {

		}

		nvePage() : deviceMemloc(0), size(0), IsReservior(0)
		{

		}


		static void ReadResivourSignature(uint32_t ResSignature, bool& isRes, bool& isSmall, bool& fast, uint32_t& position) {
			isRes = ResSignature > 0;
			isSmall = ResSignature & 1;
			fast = ResSignature & 2;
			position = (ResSignature >> 2) - 1;// && 62
		//	resivour = (ResSignature && 65472 >> 1) - 1;
		}
		//, UINT& resivour
		static uint32_t EncodeResivourSignature(int index, short resivourType) {
			return ((index + 1) << 2) + resivourType;
		}
	};
}