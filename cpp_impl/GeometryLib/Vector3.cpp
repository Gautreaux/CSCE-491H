#include "Vector3.h"

#include "math.h"

Vector3::Vector3(const double& x, const double& y, const double& z, const bool& convertToUnitVector) :
    Point3(x,y,z)
{
    double m = getMagnitude();
    if(m != 0){
        this->x /= m;
        this->y /= m;
        this->z /= m;
    }// if m == 0, everything else should also already be zero
}

Vector3::Vector3(const Point3& start, const Point3& end, const bool& convertToUnitVector) :
    Point3(start - end)
{
    double m = getMagnitude();
    if(m != 0){
        this->x /= m;
        this->y /= m;
        this->z /= m;
    }// if m == 0, everything else should also already be zero
}

double inline Vector3::getMagnitude(void) const{
    return sqrt(x*x+y*y+z*z);
}