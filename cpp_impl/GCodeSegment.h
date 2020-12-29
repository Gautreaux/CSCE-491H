#pragma once

#include "pch.h"

#include "GeometryLib/LineSegment.h"

class GCodeSegment : public LineSegment{
private:
    double printAmount;
public:
    GCodeSegment(const Point3& startPoint, const Point3& endPoint, double printAmount);

    double inline getPrintAmount(void) const {return printAmount;}
};

std::ostream &operator<<(std::ostream& os, const GCodeSegment& seg);