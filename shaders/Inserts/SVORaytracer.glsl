// #include Helpers/ChunkHeaders.glsl
// #include Helpers/GPUMemory.glsl
// #include Helpers/Rays.glsl
// #include Helpers/RayMasks.glsl
// #include ChunkGPUConst.h
// ChunkHeaders (ChunkHeader)  ChunkHeader[] ChunkHeaders;
// VoxelBrick[] GPUMemory
//  uint ptr_stack[MaxQuality+1]

// X is starts at bit 0
// Y starts at bit 2
// Z starts at bit 4, staggering is intentional allowing for individual qualities to be picked out and merged
// uint current_childPtr;




// ivec3 SVOPosition

vec3 hitAABB(vec3 chunkPosition, vec3 invDir, const Ray r)
{
  if(all(lessThan(r.position, chunkPosition + chunk_dimensions)) && all(greaterThan(r.position, chunkPosition))){
   return r.position - chunkPosition;
  }

  vec3  diff   = chunkPosition - r.position;

  vec3  tbot   = invDir * diff;
  vec3  ttop   = invDir * (diff + chunk_dimensions - vec3(1/16.0));
  vec3  tmin   = min(ttop, tbot);
  vec3  tmax   = max(ttop, tbot);
  float t0     = max(tmin.x, max(tmin.y, tmin.z));

  return r.direction * t0 + r.position - chunkPosition;
}

/*
ivec3 sign11(vec3 vec){
  
  ivec3 v = ivec3(-1);
  if(vec.x>=0)
        v.x=1;
  
  if(vec.y>=0)
        v.y=1;

  if(vec.z>=0)
        v.z=1;

   return v;
}
*/
void SetSVOPosition(vec3 position){
    SVOPosition = ivec3(position*greatestZoom);
}
void SetPtr(int current_quality_level){
    current_childPtr = ((SVOPosition.x>>(current_quality_level*2 - 2)) & 0x03) | ((SVOPosition.y>>(current_quality_level*2 - 2))& 0xC) | ((SVOPosition.z>>(current_quality_level*2 - 2))&0x30);
}


void Descend(vec3 position, inout int current_quality_level)
{
    ptr_stack[current_quality_level-1] = GPUMemory[ptr_stack[current_quality_level]].ptrs[current_childPtr];
    current_quality_level -= 1;
    SetPtr(current_quality_level);
}

void Ascend(inout int current_quality_level, int delta){
    current_quality_level += delta;
    SetPtr(current_quality_level);
}

uint GetVoxelAtPosition(vec3 position, inout int current_quality_level){
   
   VoxelBrick currentBrick;

    while (true) {

        currentBrick = GPUMemory[ptr_stack[current_quality_level]];


        // Check Voxel at Positions Mask
        // Position is empty so is filled with air
        if((currentBrick.mask & (1<<current_childPtr)) == 0){
        // Needs to move forwards
           // Ascend(current_quality_level);
            return 0;
        }

        if((currentBrick.ptrs[current_childPtr] & const_value) > 0){ // WRONG
             // Constant (Potential hit!)
             uint voxel = currentBrick.ptrs[current_childPtr] & ptr_mask;
            // Ascend(current_quality_level);
             return  voxel;
        }

        ///// DESCEND (1)
        Descend(position, current_quality_level);
    }
}

bool Step(const uint64_t ptr_mask, const vec3 inv_ray_dir, const vec3 direction, const vec3 dir_sign, const vec3 s01, inout vec3 position, inout int current_quality_level){
   
    float voxelSize = QualityToSize(current_quality_level-1);

    // intersect the xy lines
    vec3 tMax = inv_ray_dir * (voxelSize * s01 - mod(position, voxelSize));

    // Remap SVO
    SetSVOPosition(position);

    // step by the shorter distance
    position += min(min(tMax.x, tMax.y),tMax.z) * direction;

    // which line is intersected? Gets the compares .n vs the greatest of the other two
    vec3 mask = vec3(min(tMax.y, tMax.z),min(tMax.x, tMax.z), min(tMax.x, tMax.y));
    mask = step(tMax, mask);

    position += 1/64.0 * dir_sign * mask;

    // Boundry
    ivec3 boundry = ivec3((dir_sign + 1.0) * .5) * 3; //0 -> 3
    int index = int(mask.y) + int(mask.z) * 2; //mask.x * 0;
    int n = 0;

    for (int i = current_quality_level; i<=MaxQuality; i++){
        ivec3 rel_pos = (SVOPosition>>(i*2-2))&3;
        if(rel_pos[index] != boundry[index])
            break;
        n++;
    }

    // Remap SVO
    SetSVOPosition(position);
    
    // Stagger the SVOPosition for later merging
    SVOPosition.y = SVOPosition.y << 2;
    SVOPosition.z = SVOPosition.z << 4;

   // if(any(greaterThanEqual(position,vec3(32))) || any(lessThan(position,vec3(0))))
   //     return true;

    SetPtr(current_quality_level);

    Ascend(current_quality_level, n);

    // Quick Scan for Valid Blocks
    for (int j = current_quality_level; j<=MaxQuality; j++){
        VoxelBrick current = GPUMemory[ptr_stack[current_quality_level]];
    
        if((current.mask & ptr_mask) == 0){
         Ascend(current_quality_level, 1);
            continue;
        }

        // Found a valid mask
        break;
    }

    return current_quality_level > MaxQuality;
}

bool StepGetDistance(const uint64_t ptr_mask, const vec3 inv_ray_dir, const vec3 direction, const vec3 dir_sign, const vec3 s01, inout vec3 position, inout int current_quality_level, out float dist){
   
    float voxelSize = QualityToSize(current_quality_level-1);

    // intersect the xy lines
    vec3 tMax = inv_ray_dir * (voxelSize * s01 - mod(position, voxelSize));

    // Remap SVO
    SetSVOPosition(position);


    // step by the shorter distance
    dist = min(min(tMax.x, tMax.y),tMax.z);
    position += dist * direction;

    // which line is intersected? Gets the compares .n vs the greatest of the other two
    vec3 mask = vec3(min(tMax.y, tMax.z),min(tMax.x, tMax.z), min(tMax.x, tMax.y));
    mask = step(tMax, mask);

    position += 1/64.0 * dir_sign * mask;

    // Boundry
    ivec3 boundry = ivec3((dir_sign + 1.0) * .5) * 3; //0 -> 3
    int index = int(mask.y) + int(mask.z) * 2; //mask.x * 0;
    int n = 0;

    for (int i = current_quality_level; i<=MaxQuality; i++){
        ivec3 rel_pos = (SVOPosition>>(i*2-2))&3;
        if(rel_pos[index] != boundry[index])
            break;
        n++;
    }

    // Remap SVO
    SetSVOPosition(position);
    
    // Stagger the SVOPosition for later merging
    SVOPosition.y = SVOPosition.y << 2;
    SVOPosition.z = SVOPosition.z << 4;

   // if(any(greaterThanEqual(position,vec3(32))) || any(lessThan(position,vec3(0))))
   //     return true;

    SetPtr(current_quality_level);

    Ascend(current_quality_level, n);

    // Quick Scan for Valid Blocks
    for (int j = current_quality_level; j<=MaxQuality; j++){
        VoxelBrick current = GPUMemory[ptr_stack[current_quality_level]];
    
        if((current.mask & ptr_mask) == 0){
         Ascend(current_quality_level, 1);
            continue;
        }

        // Found a valid mask
        break;
    }

    return current_quality_level > MaxQuality;
}

bool Raytrace(Ray ray, const TracingPackage info, uint volume, uint chunkptr, out RayHit hit){

   // ChunkHeader header = ChunkHeaders[volume];


    if (ChunkHeaders[volume].ptrs[chunkptr] == MAXUINT)
        return false;

         
    if (GPUMemory[ChunkHeaders[volume].ptrs[chunkptr]].mask == 0)
        return false;


    vec3 position = hitAABB((ChunkHeaders[volume].position + GetHeaderOffset(chunkptr))* chunk_dimensions, info.inv_dir, ray);
    position += ray.direction * 1/64.0;


    int current_quality_level = MaxQuality;


   // float scale = minimumResolution >> 2; // div 4
    
    SetSVOPosition(position);
    // Stagger the SVOPosition for later merging
    SVOPosition.y = SVOPosition.y << 2;
    SVOPosition.z = SVOPosition.z << 4;

    ptr_stack[current_quality_level] = ChunkHeaders[volume].ptrs[chunkptr];
    SetPtr(current_quality_level);
    

    
    hit.position = position;

    
    const uint64_t mask = GetMask(current_childPtr, info.s_dir);


    if((GPUMemory[ChunkHeaders[volume].ptrs[chunkptr]].mask & mask) == 0)
        return false;

    int n = 0;
    while (true){

        uint voxel = GetVoxelAtPosition(position, current_quality_level);
        if (voxel > 0){

            if(any(greaterThanEqual(position,chunk_dimensions)) || any(lessThan(position,vec3(0))) ){
                return false;
            }
            position += (ChunkHeaders[volume].position + GetHeaderOffset(chunkptr)) * chunk_dimensions;
            hit._distance = distance(position,ray.position);
            //hit.position = vec3(0,1,n);
            hit.position = position;
            hit.blockID = voxel;
            return true;
        }


   

        if(Step(mask, info.inv_dir, ray.direction, info.s_dir, info.s01, position, current_quality_level)){
         hit.position = vec3(0,1,n);
         return  false;
        }
        n++;
    }
}



///// DISTANCE TRACER ////////////////////////////////////////


bool RaytraceDistanceLimited_ConeTrace_World(Ray ray, const TracingPackage info, const int RaySize, uint volume, uint chunkptr, inout float currentDistance, const float maxDistance, out RayHit hit){

   // ChunkHeader header = ChunkHeaders[volume];


    if (ChunkHeaders[volume].ptrs[chunkptr] == MAXUINT)
        return false;

         
    if (GPUMemory[ChunkHeaders[volume].ptrs[chunkptr]].mask == 0)
        return false;


    vec3 position = hitAABB((ChunkHeaders[volume].position + GetHeaderOffset(chunkptr))* chunk_dimensions, info.inv_dir, ray);
    position += ray.direction * 1/64.0;


    int current_quality_level = MaxQuality;


   // float scale = minimumResolution >> 2; // div 4
    
    SetSVOPosition(position);
    // Stagger the SVOPosition for later merging
    SVOPosition.y = SVOPosition.y << 2;
    SVOPosition.z = SVOPosition.z << 4;

    ptr_stack[current_quality_level] = ChunkHeaders[volume].ptrs[chunkptr];
    SetPtr(current_quality_level);
    

    
    hit.position = position;

    
    const uint64_t mask = GetMask(current_childPtr, info.s_dir);


    if((GPUMemory[ChunkHeaders[volume].ptrs[chunkptr]].mask & mask) == 0)
        return false;

    int n = 0;
    while (true){

        uint voxel = GetVoxelAtPosition(position, current_quality_level);
        if (voxel > 0){

            if(any(greaterThanEqual(position,chunk_dimensions)) || any(lessThan(position,vec3(0))) ){
                return false;
            }

            position += (ChunkHeaders[volume].position + GetHeaderOffset(chunkptr)) * chunk_dimensions;
            hit._distance = distance(position,ray.position);
            //hit.position = vec3(0,1,n);
            hit.position = position;
            hit.blockID = voxel;
            return true;
        }

        if(StepGetDistance(mask, info.inv_dir, ray.direction, info.s_dir, info.s01, position, current_quality_level, dist)){
             hit.position = vec3(0,1,n);
             return  false;
        }
        
        currentDistance += dist;

        if(dist> maxDistance)
            return false;

        n++;
    }
}



bool RaytraceDistanceLimited(Ray ray, const TracingPackage info, uint volume, uint chunkptr, vec3 position, inout float currentDistance, const float maxDistance, out RayHit hit){

   // ChunkHeader header = ChunkHeaders[volume];


    if (ChunkHeaders[volume].ptrs[chunkptr] == MAXUINT)
        return false;

         
    if (GPUMemory[ChunkHeaders[volume].ptrs[chunkptr]].mask == 0)
        return false;


    position += ray.direction * 1/64.0;

    int current_quality_level = MaxQuality;


   // float scale = minimumResolution >> 2; // div 4
    
    SetSVOPosition(position);
    // Stagger the SVOPosition for later merging
    SVOPosition.y = SVOPosition.y << 2;
    SVOPosition.z = SVOPosition.z << 4;

    ptr_stack[current_quality_level] = ChunkHeaders[volume].ptrs[chunkptr];
    SetPtr(current_quality_level);
    

    
    hit.position = position;

    
    const uint64_t mask = GetMask(current_childPtr, info.s_dir);


    if((GPUMemory[ChunkHeaders[volume].ptrs[chunkptr]].mask & mask) == 0)
        return false;

    int n = 0;
    float dist;
    while (true){

        uint voxel = GetVoxelAtPosition(position, current_quality_level);
        if (voxel > 0){

            if(any(greaterThanEqual(position,chunk_dimensions)) || any(lessThan(position,vec3(0))) ){
                return false;
            }
            position += (ChunkHeaders[volume].position + GetHeaderOffset(chunkptr)) * chunk_dimensions;
            hit._distance = distance(position,ray.position);
            //hit.position = vec3(0,1,n);
            hit.position = position;
            hit.blockID = voxel;
            return true;
        }


   

        if(StepGetDistance(mask, info.inv_dir, ray.direction, info.s_dir, info.s01, position, current_quality_level, dist)){
         hit.position = vec3(0,1,n);
         return  false;
        }

        currentDistance += dist;

        if(dist> maxDistance)
            return false;

        n++;
    }
}

