#pragma once

#include "../pch.h"

class Point3{
protected:
    double x;
    double y;
    double z;

public:
    Point3(const double& x, const double& y, const double& z);
    Point3(const Point3& other);

    inline double getX(void) const {return x;}
    inline double getY(void) const {return y;}
    inline double getZ(void) const {return z;}
};

inline bool operator==(const Point3& lhs, const Point3& rhs){
    if(lhs.getX() != rhs.getX()){
        return false;
    }
    if(lhs.getY() != rhs.getY()){
        return false;
    }
    if(lhs.getZ() != rhs.getZ()){
        return false;
    }
    return true;
}

inline bool operator!=(const Point3& lhs, const Point3& rhs){
    return !(lhs == rhs);
}

inline Point3 operator-(const Point3& lhs, const Point3& rhs){
    return Point3(lhs.getX() - rhs.getX(), lhs.getY() - rhs.getY(), lhs.getZ() - rhs.getZ());
}

std::ostream &operator<<(std::ostream& os, const Point3& point);