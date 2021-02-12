#pragma once

#include "../pch.h"

#include "math.h" //for pow, sqrt

class Point3{
protected:
    double x;
    double y;
    double z;

public:
    Point3(void);
    Point3(const double& x, const double& y, const double& z);
    Point3(const Point3& other);

    inline double getX(void) const {return x;}
    inline double getY(void) const {return y;}
    inline double getZ(void) const {return z;}
};

inline bool operator==(const Point3& lhs, const Point3& rhs){
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

inline bool operator!=(const Point3& lhs, const Point3& rhs){
    return !(lhs == rhs);
}

inline bool operator<(const Point3& lhs, const Point3& rhs){
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

inline Point3 operator-(const Point3& lhs, const Point3& rhs){
    return Point3(lhs.getX() - rhs.getX(), lhs.getY() - rhs.getY(), lhs.getZ() - rhs.getZ());
}

std::ostream &operator<<(std::ostream& os, const Point3& point);

inline double getPointDistance(const Point3& p1, const Point3& p2){
    return sqrt(pow(p2.getX() - p1.getX(), 2) + pow(p2.getY() - p1.getY(), 2) + pow(p2.getZ() - p1.getZ(), 2));
}

//scalar addition
inline Point3 operator+(const Point3& lhs, const Point3& rhs){
    return Point3(lhs.getX() + rhs.getX(), lhs.getY() + rhs.getY(), lhs.getZ() + rhs.getZ());
}

//scalar mult
inline Point3 operator*(const Point3& lhs, const Point3& rhs){
    return Point3(lhs.getX() * rhs.getX(), lhs.getY() * rhs.getY(), lhs.getZ() * rhs.getZ());
}
inline Point3 operator*(const Point3& lhs, const double rhs){
    return Point3(lhs.getX() * rhs, lhs.getY() * rhs, lhs.getZ() * rhs);
}
inline Point3 operator*(const double lhs, const Point3& rhs){
    return rhs * lhs;
}