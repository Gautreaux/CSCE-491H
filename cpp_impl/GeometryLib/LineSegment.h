#pragma once

#include "../pch.h"

#include "Line.h"
#include "Point3.h"

class LineSegment : public Line{
protected:
    Point3 endPoint;
public:
    //LineSegment(const Point3& point, const Slope& slope);
    LineSegment(const Point3& point, const Point3& otherPoint);

    const Point3& getStartPoint(void) const {return point;}
    const Point3& getEndPoint(void) const {return endPoint;}
};

std::ostream &operator<<(std::ostream& os, const LineSegment& seg);