#pragma once

#include "../pch.h"

#include "Point3.h"

class Vector3 : public Point3{
public:
    Vector3(const double& x, const double& y, const double& z, const bool convertToUnitVector=true);
    Vector3(const Point3& start, const Point3& end, const bool convertToUnitVector=true);
    Vector3(const Point3& p, const bool convertToUnitVector=true);

    //return the magnitude of this vector
    double inline getMagnitude(void) const{
        return sqrt(x*x+y*y+z*z);
    }

    //return the reverse of this vector
    Vector3 inline reverse(void) const{
        return (*(this)) * -1;
    }
};


inline bool operator==(const Vector3& lhs, const Vector3& rhs){
    return operator==((const Point3&)lhs, (const Point3&)rhs);
}

//scalar mult, probably equivalent to Point3 operators
//  these do not unitize
inline Vector3 operator*(const Vector3& lhs, const Vector3& rhs){
    return Vector3(lhs.getX() * rhs.getX(), lhs.getY() * rhs.getY(), lhs.getZ() * rhs.getZ(), false);
}
inline Vector3 operator*(const Vector3& lhs, const double rhs){
    return Vector3(lhs.getX() * rhs, lhs.getY() * rhs, lhs.getZ() * rhs, false);
}
inline Vector3 operator*(const double lhs, const Vector3& rhs){
    return rhs * lhs;
}

inline double dot_product(const Vector3& lhs, const Vector3& rhs){
    return (lhs.getX() * rhs.getX()) + (lhs.getY() * rhs.getY()) + (lhs.getZ() * rhs.getZ());
}

inline Vector3 cross_product(const Vector3& lhs, const Vector3& rhs, bool convertToUnitVector = false){
    return Vector3(
        lhs.getY()*rhs.getZ() - lhs.getZ()*rhs.getY(),
        lhs.getZ()*rhs.getX() - lhs.getX()*rhs.getZ(),
        lhs.getX()*rhs.getY() - lhs.getY()*rhs.getX(),
        convertToUnitVector
    );
}