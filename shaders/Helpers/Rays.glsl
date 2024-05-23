struct Ray{
	vec3 position;
	vec3 direction;
};

struct RayHit{
	vec3 position;
	uint blockID;
	vec3 normal;
	float _distance;
	vec3 dir;
};

struct TracingPackage{
    vec3 s_dir;
    vec3 inv_dir;
    vec3 s01;
};

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


TracingPackage CreateTracingPackage(Ray ray){
    TracingPackage package;

    package.s_dir = sign11(ray.direction);
    package.inv_dir = 1/ray.direction;
    package.s01 = max(package.s_dir, 0.);

    return package;
}
*/