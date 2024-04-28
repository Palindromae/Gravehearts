#include "stdafx.h"
#include "DistanceCalculation.h"

float manhattenDistance(glm::ivec3 a) {
	return abs(a.x) + abs(a.y) + abs(a.z);
}
float manhattenDistance(glm::vec3 a) {
	return abs(a.x) + abs(a.y) + abs(a.z);
}
float EuclideanDistance(glm::vec3 a) {
	return glm::sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
}
float manhattenDistance(glm::vec2 a) {
	return abs(a.x) + abs(a.y);
}
//Returns maximum distance
float ChebyshevDistance(const glm::ivec3 a) {
	return std::max(std::max(abs(a.x), abs(a.y)), abs(a.z));
}
float ChebyshevDistance(const glm::vec3 a) {
	return std::max(std::max(abs(a.x), abs(a.y)), abs(a.z));
}
float MaximumDistance(const glm::ivec3 a) {
	return ChebyshevDistance(a);
}