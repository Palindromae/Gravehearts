struct Ray{
	vec3 position;
	vec3 direction;
};

struct RayHit{
	vec3 position;
	uint blockID;
	vec3 normal;
	float _distance;
};