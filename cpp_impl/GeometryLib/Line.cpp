#include "Line.h"

Line::Line(const Point3& point, const Slope& slope) : point(point), slope(slope){};

Line::Line(const Point3& point, const Point3& otherPoint) :
    point(point), slope(point, otherPoint)
{
    //TODO - do this one too
    // is this done though?
}