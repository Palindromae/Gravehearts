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

#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_buffer_reference2 : require

#include "raycommon.glsl"
#include "wavefront.glsl"
#include "Helpers/Rays.glsl"

hitAttributeEXT vec4 attribs;

// clang-format off
layout(location = 0) rayPayloadInEXT RayResponse prd;
//layout(location = 1) rayPayloadEXT bool isShadowed;


//layout(buffer_reference, scalar) buffer Vertices {Vertex v[]; }; // Positions of an object
//layout(buffer_reference, scalar) buffer Indices {uint i[]; }; // Triangle indices
//layout(buffer_reference, scalar) buffer Materials {WaveFrontMaterial m[]; }; // Array of all materials on an object
//layout(buffer_reference, scalar) buffer MatIndices {int i[]; }; // Material ID for each triangle
//layout(set = 1, binding = eObjDescs, scalar) buffer ObjDesc_ { ObjDesc i[]; } objDesc;

layout(push_constant) uniform _PushConstantRay { PushConstantRay pcRay; };
// clang-format on


layout(location = 3) callableDataEXT rayLight cLight;


void main()
{
  vec3 worldPos = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;

  RayResponse resp;
  resp.hit._distance = attribs.w;
  resp.albedo = attribs.xyz;
  prd = resp;
}
