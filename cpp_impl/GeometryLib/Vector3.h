#pragma once

#include "../pch.h"

#include "Point3.h"

class Vector3 : public Point3{
public:
    Vector3(const double& x, const double& y, const double& z, const bool& convertToUnitVector=true);
    Vector3(const Point3& start, const Point3& end, const bool& convertToUnitVector=true);

    //return the magnitude of this vector
    double inline getMagnitude(void) const;
};