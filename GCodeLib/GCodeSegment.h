#pragma once

#include "../pch.h"

#include "../GeometryLib/LineSegment.cuh"

class GCodeSegment : public LineSegment{
private:
    double printAmount;
public:
    GCodeSegment(const Point3& startPoint, const Point3& endPoint, double printAmount);

    double inline getPrintAmount(void) const {return printAmount;}

    //TODO - is a negative print amount a print segment?
    bool inline isPrintSegment(void) const {return printAmount > 0;}

#ifdef DEBUG
    void print(void) {std::cout << this << std::endl;}
#endif
};

std::ostream &operator<<(std::ostream& os, const GCodeSegment& seg);