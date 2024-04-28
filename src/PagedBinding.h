#pragma once
#include "nve_page.h"
namespace nve {
	struct nvePagedBinding {
	public:
		nvePage** pages;
		long pageCount;
		long long size;

		~nvePagedBinding() {
			delete pages;
		}
		

	};
}