#include "GCodeSegment.h"

GCodeSegment::GCodeSegment(const Point3& startPoint, const Point3& endPoint, double printAmount) :
    LineSegment(startPoint, endPoint)
{
    this->printAmount = printAmount;
}


std::ostream &operator<<(std::ostream& os, const GCodeSegment& seg){
    os << static_cast<const LineSegment&>(seg) << " " << seg.getPrintAmount();
    return os;
};