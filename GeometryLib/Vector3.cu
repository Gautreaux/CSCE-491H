#include "Vector3.h"

#include "math.h"

Vector3::Vector3(const double& x, const double& y, const double& z, const bool convertToUnitVector) :
    Point3(x,y,z)
{
    if(!convertToUnitVector){
        return;
    }

    double m = getMagnitude();
    if(m != 0){
        this->x /= m;
        this->y /= m;
        this->z /= m;
    }// if m == 0, everything else should also already be zero
}

Vector3::Vector3(const Point3& start, const Point3& end, const bool convertToUnitVector) :
    Point3(end - start)
{
    if(!convertToUnitVector){
        return;
    }

    double m = getMagnitude();
    if(m != 0){
        this->x /= m;
        this->y /= m;
        this->z /= m;
    }// if m == 0, everything else should also already be zero
}

Vector3::Vector3(const Point3& p, const bool convertToUnitVector) :
    Point3(p)
{
    if(!convertToUnitVector){
        return;
    }

    double m = getMagnitude();
    if(m != 0){
        this->x /= m;
        this->y /= m;
        this->z /= m;
    }// if m == 0, everything else should also already be zero
}