#pragma once

#include "../pch.h"

#include "Point3.cuh"
#include "Vector3.cuh"

typedef Vector3 Slope;

class Line{
protected:
    Point3 point;
    Slope slope;

public:
    NVCC_HD Line(const Point3& point, const Slope& slope);
    NVCC_HD Line(const Point3& point, const Point3& otherPoint);

    NVCC_HD inline const Point3& getPoint(void) const {return point;}
    NVCC_HD inline const Slope& getSlope(void) const {return slope;}

    // return true iff segment is parallel to z = 0
    NVCC_HD bool inline isZParallel(void) const {return DOUBLE_EQUAL(slope.getZ(), 0);}

    //line comparison functions
    NVCC_HD bool isParallel(const Line& other) const;
    NVCC_HD bool isCollinear(const Line& other) const;
    // bool isSkew(const Line& other) const;

    //return true iff point is on this line
    NVCC_HD bool isOnLine(const Point3& testPoint) const;

    class NoIntersectionException : public std::exception{};
    class CollinearIntersectionException : public std::exception{};

    //get the point where the lines intersect, 
    //  raising an exception if they do not, or are collinear
    NVCC_HD Point3 getLineIntersectPoint(const Line& other) const;

    // return the projection of testPoint onto this line
    NVCC_HD Point3 getProjectionPoint(const Point3& testPoint) const;
};

std::ostream& operator<<(std::ostream& os, const Line& line);