#pragma once

#include "../pch.h"

#include <limits> // for numeric_limit
#include <math.h> //for pow, sqrt

class Point3{
protected:
    double x;
    double y;
    double z;

public:
    NVCC_HD Point3(void);
    NVCC_HD Point3(const double& x, const double& y, const double& z);
    NVCC_HD Point3(const Point3& other);

    NVCC_HD inline double getX(void) const {return x;}
    NVCC_HD inline double getY(void) const {return y;}
    NVCC_HD inline double getZ(void) const {return z;}

    //conceptual point representing any point
    const static Point3 ANY;
};

NVCC_HD inline bool operator==(const Point3& lhs, const Point3& rhs){
    if(DOUBLE_NEQ(lhs.getX(), rhs.getX())){
        return false;
    }
    if(DOUBLE_NEQ(lhs.getY(), rhs.getY())){
        return false;
    }
    if(DOUBLE_NEQ(lhs.getZ(), rhs.getZ())){
        return false;
    }
    return true;
}

NVCC_HD inline bool operator!=(const Point3& lhs, const Point3& rhs){
    return !(lhs == rhs);
}

NVCC_HD inline bool operator<(const Point3& lhs, const Point3& rhs){
    if(DOUBLE_LT(lhs.getX(), rhs.getX())){
        return true;
    }else if(DOUBLE_GT(lhs.getX(), rhs.getX())){
        return false;
    }  
    //x dim is equal

    if(DOUBLE_LT(lhs.getY(), rhs.getY())){
        return true;
    }else if(DOUBLE_GT(lhs.getY(), rhs.getY())){
        return false;
    }
    //y dim is also equal

    return DOUBLE_LT(lhs.getZ(), rhs.getZ());
}

NVCC_HD inline Point3 operator-(const Point3& lhs, const Point3& rhs){
    return Point3(lhs.getX() - rhs.getX(), lhs.getY() - rhs.getY(), lhs.getZ() - rhs.getZ());
}

std::ostream &operator<<(std::ostream& os, const Point3& point);

NVCC_HD inline double getPointDistance(const Point3& p1, const Point3& p2){
    return sqrt(pow(p2.getX() - p1.getX(), 2) + pow(p2.getY() - p1.getY(), 2) + pow(p2.getZ() - p1.getZ(), 2));
}

//scalar addition
NVCC_HD inline Point3 operator+(const Point3& lhs, const Point3& rhs){
    return Point3(lhs.getX() + rhs.getX(), lhs.getY() + rhs.getY(), lhs.getZ() + rhs.getZ());
}

//scalar mult
NVCC_HD inline Point3 operator*(const Point3& lhs, const Point3& rhs){
    return Point3(lhs.getX() * rhs.getX(), lhs.getY() * rhs.getY(), lhs.getZ() * rhs.getZ());
}
NVCC_HD inline Point3 operator*(const Point3& lhs, const double rhs){
    return Point3(lhs.getX() * rhs, lhs.getY() * rhs, lhs.getZ() * rhs);
}
NVCC_HD inline Point3 operator*(const double lhs, const Point3& rhs){
    return rhs * lhs;
}