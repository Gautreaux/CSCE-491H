#include "LineSegment.h"

LineSegment::LineSegment(const Point3& point, const Point3& otherPoint) :
    Line(point, otherPoint), endPoint(otherPoint)
{
}

std::ostream &operator<<(std::ostream& os, const LineSegment& seg){
    os << seg.getStartPoint() << " -> " << seg.getEndPoint();
    return os;
}