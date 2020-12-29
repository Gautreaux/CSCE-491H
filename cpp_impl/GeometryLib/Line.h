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

    // return true iff segment is parallel to z = 0
    bool inline isZParallel(void) const {return slope.getZ() == 0;}
};