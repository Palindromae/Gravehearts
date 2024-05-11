/*
 * Copyright (c) 2019-2021, NVIDIA CORPORATION.  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-FileCopyrightText: Copyright (c) 2019-2021 NVIDIA CORPORATION
 * SPDX-License-Identifier: Apache-2.0
 */


#ifdef __cplusplus
#pragma once
#include <cstdint>
 // GLSL Type
using uint = unsigned int;
#endif

// clang-format off
#ifdef __cplusplus // Descriptor binding helper for C++ and GLSL
#define START_BINDING(a) enum a {
#define END_BINDING() }
#else
#define START_BINDING(a)  const uint
#define END_BINDING() 
#endif

struct VoxelBrick { // make sure to adjust chapter_size if any of this is change to the size of the new struct
	uint64_t mask;
	uint blockID;
	uint ptrs[64];
};


#ifdef __cplusplus
namespace nve::GPUMemory {
#endif

	const int chapters = 512; // must be a power of two to hash
	const int TotalMemory = 512000000; //bytes (512mb) Make sure any changes are compatible with VoxelBrick size
	const int chapter_size = TotalMemory / chapters / 272;
	const int page_size = 16; // 8 nodes two ints
	const int header_size = 1;
	const uint k_prime = 7919u;

	const int ptr_mask = 2147483647;

	//const int leaf_value = 1 << 30;
	const int const_value = 1 << 31;
	//const int const_leaf_value = 3 << 30;



	struct UnexpandedNode {
		int position; // 8 bits for x then y then z
		uint mempos;
	};

#ifdef __cplusplus
}
#endif