// #include Helpers/ChunkHeaders.glsl
// #include Helpers/GPUMemory.glsl
// #include Helpers/Rays.glsl
// #include Helpers/RayMasks.glsl
// ChunkHeaders (ChunkHeader)  ChunkHeader[] ChunkHeaders;
// VoxelBrick[] GPUMemory
// uint ptr_stack[MaxQuality]

// ivec3 SVOPosition
// X is starts at bit 0
// Y starts at bit 2
// Z starts at bit 4, staggering is intentional allowing for individual qualities to be picked out and merged
// uint current_childPtr;

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

void SetSVOPosition(vec3 position){
    SVOPosition = ivec3(position*greatestZoom);
}
void SetPtr(int current_quality_level){
    current_childPtr = ((SVOPosition.x>>(current_quality_level*2)) & 0x03) | ((SVOPosition.y>>(current_quality_level*2))& 0xC) | ((SVOPosition.z>>(current_quality_level*2))&0x30);
    int a =0;
}


void Descend(vec3 position, inout int current_quality_level)
{
    ptr_stack[current_quality_level-1] = GPUMemory[ptr_stack[current_quality_level]].ptrs[current_childPtr];
    current_quality_level -= 1;
    SetPtr(current_quality_level);
}

void Ascend(inout int current_quality_level){
    current_quality_level += 1;
    SetPtr(current_quality_level);
}

uint GetVoxelAtPosition(uint64_t mask,  vec3 position, inout int current_quality_level){
    VoxelBrick currentBrick = GPUMemory[ptr_stack[current_quality_level]];

    // Check Voxel at Positions Mask
    // Position is empty so is filled with air
    if((currentBrick.mask & (1<<current_childPtr)) == 0){
    // Needs to move forwards
      return 0;
    }

    while (true) {

        if((currentBrick.ptrs[current_childPtr] & const_value) > 0){ // WRONG
            // Constant (Potential hit!)
            return  currentBrick.ptrs[current_childPtr] & ptr_mask;
        }
    
        ///// DESCEND (1)
        Descend(position, current_quality_level);

        currentBrick = GPUMemory[ptr_stack[current_quality_level]];

        if((mask & currentBrick.mask) == 0)
        {
            Ascend(current_quality_level);
            return 0;
        }
    }
}

bool Step(vec3 inv_ray_dir, vec3 dir_signed, vec3 s01, inout vec3 position, inout int current_quality_level){
   
        float voxelSize = QualityToSize(current_quality_level);

        // intersect the xy lines
        vec3 tMax = inv_ray_dir * (voxelSize * s01 - mod(position, voxelSize));

        // step by the shorter distance
        position += min(min(tMax.x, tMax.y),tMax.z) * dir_signed;


        // which line is intersected? Gets the compares .n vs the greatest of the other two
        vec3 mask = vec3(max(tMax.y, tMax.z),max(tMax.x, tMax.z), max(tMax.x, tMax.y));
        mask = step(tMax,mask);// step(tMax.xy, tMax.yx)

        position += 4e-4 * dir_signed * mask;

        SetSVOPosition(position);
       
       // Boundry
        ivec3 relative_position;

        do{
             relative_position = (SVOPosition>>(current_quality_level*2))&3;
             relative_position += ivec3(mask * dir_signed);
        
             // Check the boundries 4 and -1 are beyond the boundry
              
             if(any(equal(((~(relative_position))&ivec3(4)), ivec3(0)))){
                //ascend
                Ascend(current_quality_level);
                if(current_quality_level>=MaxQuality)
                    return true;
                continue;
             }
             break;
        } while(true);

        // Stagger the SVOPosition for later merging
        SVOPosition.y = SVOPosition.y << 2;
        SVOPosition.z = SVOPosition.z << 4;

        return false;
}

bool Raytrace(Ray ray, uint chunk, uint chunkptr, out RayHit hit){

    ChunkHeader header = ChunkHeaders[chunk];

    VoxelBrick current = GPUMemory[header.ptrs[chunkptr]];

    if (current.mask == 0)
        return false;


    vec3 s_dir = sign11(ray.direction);
    vec3 inv_dir = 1/ray.direction;
    vec3 position = ray.position - header.position * chunk_dimensions;
    vec3 s01 = max(s_dir, 0.);

    int current_quality_level = MaxQuality;

   // float scale = minimumResolution >> 2; // div 4
    
    SetSVOPosition(position);
    Descend(position, current_quality_level);
  
    uint64_t mask = GetMask(current_childPtr, ray.direction);

    if((current.mask & mask) == 0)
        return false;

    while (true){

       uint voxel = GetVoxelAtPosition(mask, position, current_quality_level);

       if (voxel > 0){
          hit.position = position; //possibly wrong
          hit.blockID = voxel;
          return true;
       }

        if(Step(inv_dir, s_dir, s01, position, current_quality_level)){
            return false;
        }

    }
}