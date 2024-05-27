


//const float invRoot = 1/sqrt(2.0);
float interpolate(float xM, float xP,  float yM, float yP, float zM, float zP,float stepSize, vec3 posRelStep)
{
xM += (xP - xM) * (posRelStep.x) / stepSize;
yM += (yP - yM) * (posRelStep.y) / stepSize;
zM += (zP - zM) * (posRelStep.z) / stepSize;


return (xM + yM + zM)*.333333;



}

float interpolate(float xM, float xP, float yM, float yP, float stepSize, vec2 posRelStep)
{

xM += (xP - xM) * (posRelStep.x)/stepSize;
yM += (yP - yM) * (posRelStep.y)/stepSize;

return (xM + yM) * .5;
}

float bilinearInterpolation(vec2 self,vec2 xyM, vec2 xyP, float xMyM, float xMyP, float xPyM, float xPyP)
{

float D0 = 1.0 / (xyP.x - xyM.x);

float D1 = (xyP.x-self.x) * D0;
float D2 = (self.x - xyM.x) * D0;
float R1 = D1 * xMyM + D2 * xPyM;
float R2 = D1 * xMyP + D2 * xPyP;


D0 = 1.0 /(xyP.y-xyM.y);

return (xyP.y - self.y)*D0 *R1 + (self.y-xyM.y)*D0 * R2;

}


float trilinearInterpolation(vec3 self, vec3 xyzM, vec3 xyzP, float xMyMzM, float xMyMzP,float xMyPzM,float xMyPzP,float xPyMzM, float xPyMzP,float xPyPzM,float xPyPzP)
{

float xd =  (self.x - xyzM.x)/(xyzP.x-xyzM.x);
float yd =  (self.y - xyzM.y)/(xyzP.y-xyzM.y);
float zd =  (self.z - xyzM.z)/(xyzP.z-xyzM.z);

float c00  = xMyMzM * (1-xd) + xPyMzM * xd;
float c01  = xMyMzP * (1-xd) + xPyMzP * xd;
float c10  = xMyPzM * (1-xd) + xPyPzM * xd;
float c11  = xMyPzP * (1-xd) + xPyPzP * xd;

float c0 = c00 * (1-yd) + c10 * yd;
float c1 = c01 * (1-yd) + c11 * yd;

return c0*(1-zd) + c1 *zd;

}
