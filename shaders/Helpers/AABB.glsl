// Requries
// #include "Rays.glsl"
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

bool hitAABBWithSafety(vec3 chunkPosition, vec3 invDir, const Ray r, out vec3 hit, out float dist)
{
  if(all(lessThan(r.position, chunkPosition + chunk_dimensions)) && all(greaterThan(r.position, chunkPosition))){ // Is ray within the cube
   hit = r.position - chunkPosition;
   return true;
  }

  vec3  diff   = chunkPosition - r.position;

  vec3  tbot   = invDir * diff;
  vec3  ttop   = invDir * (diff + chunk_dimensions - vec3(1/16.0));
  vec3  tmin   = min(ttop, tbot);
  vec3  tmax   = max(ttop, tbot);
  float t0     = max(tmin.x, max(tmin.y, tmin.z));
  float t1     = min(tmax.x, min(tmax.y, tmax.z));

  hit = r.direction * t0 + r.position - chunkPosition;
  dist = t0; // this all needs to be verfied
  return t0 < t1;
}