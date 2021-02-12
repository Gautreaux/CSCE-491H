#include "Point3.h"

Point3::Point3(void) : x(0), y(0), z(0)
{
}

Point3::Point3(const double& x, const double& y, const double& z) :
    x(x), y(y), z(z)
{
}

Point3::Point3(const Point3& other) :
    x(other.getX()), y(other.getY()), z(other.getZ())
{
}

std::ostream &operator<<(std::ostream& os, const Point3& point){
    os << "(" << point.getX() << ", " << point.getY() << ", " << point.getZ() << ")";
    return os;
}