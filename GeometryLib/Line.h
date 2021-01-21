#pragma once

#include "../pch.h"

#include "Point3.h"
#include "Vector3.h"

typedef Vector3 Slope;

class Line{
protected:
    Point3 point;
    Slope slope;

public:
    Line(const Point3& point, const Slope& slope);
    Line(const Point3& point, const Point3& otherPoint);

    inline const Point3& getPoint(void) const {return point;}
    inline const Slope& getSlope(void) const {return slope;}

    // return true iff segment is parallel to z = 0
    bool inline isZParallel(void) const {return DOUBLE_EQUAL(slope.getZ(), 0);}

    //line comparison functions
    bool isParallel(const Line& other) const;
    bool isCollinear(const Line& other) const;
    // bool isSkew(const Line& other) const;

    //return true iff point is on this line
    bool isOnLine(const Point3& testPoint) const;

    class NoIntersectionException : public std::exception{};
    class CollinearIntersectionException : public std::exception{};

    //get the point where the lines intersect, 
    //  raising an exception if they do not, or are collinear
    Point3 getLineIntersectPoint(const Line& other) const;

    // return the projection of testPoint onto this line
    Point3 getProjectionPoint(const Point3& testPoint) const;
};

std::ostream& operator<<(std::ostream& os, const Line& line);