
vec3 hitAABB(const vec3 volumePosition, const vec3 invDir, const Ray r)
{

   if(all(lessThan(r.position, volumePosition + chunk_dimensions)) && all(greaterThan(r.position, volumePosition))){
    return r.position;
   }
 

  vec3  diff   = volumePosition - r.position;

  vec3  tbot   = invDir * diff;
  vec3  ttop   = invDir * (diff + chunk_dimensions * NoChunksPerAxii - vec3(1/16.0));
  vec3  tmin   = min(ttop, tbot);
  vec3  tmax   = max(ttop, tbot);
  float t0     = max(tmin.x, max(tmin.y, tmin.z));

  return r.direction * t0 + r.position - volumePosition;
}



vec3 GetEntry(int volumeID, const vec3 invDir, const Ray r){
    ChunkHeader header = ChunkHeaders[volumeID];
    return hitAABB(header.position*chunk_dimensions, invDir,  r);
}

bool TraceNext(Ray r, inout vec3 position, const TracingPackage tracing_info, out int NextIndex){
    float voxelSize = chunk_dimensions.x;

    // intersect the xy lines
    vec3 tMax = tracing_info.inv_dir * (voxelSize * tracing_info.s01 - mod(position, voxelSize));

    // step by the shorter distance
    position += min(min(tMax.x, tMax.y),tMax.z) * r.direction;

    // which line is intersected? Gets the compares .n vs the greatest of the other two
    vec3 mask = vec3(min(tMax.y, tMax.z),min(tMax.x, tMax.z), min(tMax.x, tMax.y));
    mask = step(tMax, mask);
    int index = int(mask.y) + int(mask.z) * 2; //mask.x * 0;
    position += 1/32.0 * tracing_info.s_dir * mask;


    NextIndex = ChunkHeaderIndex(ivec3(position));
       
    return !(position[index] <0 || position[index]>=chunk_dimensions.x * NoChunksPerAxii);

}

bool TraceNext(Ray r, inout vec3 position, const TracingPackage tracing_info, out int NextIndex, out vec3 mask){
    float voxelSize = chunk_dimensions.x;

    // intersect the xy lines
    vec3 tMax = tracing_info.inv_dir * (voxelSize * tracing_info.s01 - mod(position, voxelSize));

    // step by the shorter distance
    position += min(min(tMax.x, tMax.y),tMax.z) * r.direction;



    // which line is intersected? Gets the compares .n vs the greatest of the other two
    mask = vec3(min(tMax.y, tMax.z),min(tMax.x, tMax.z), min(tMax.x, tMax.y));
    mask = step(tMax, mask);
    int index = int(mask.y) + int(mask.z) * 2; //mask.x * 0;
    
    position += 1/32.0 * tracing_info.s_dir * mask;


    NextIndex = ChunkHeaderIndex(ivec3(position));
       

    mask = min(min(tMax.x, tMax.y),tMax.z) * r.direction;
    return !(position[index] <0 || position[index]>=chunk_dimensions.x * NoChunksPerAxii);

}