#pragma once

#include "../pch.h"

#include "Line.h"
#include "Point3.h"

class LineSegment : public Line{
protected:
    Point3 endPoint;

    //return the distance between the enspoints between segments that are closets
    NVCC_HD double minEndPointPairDistance(const LineSegment& other) const;
public:
    //LineSegment(const Point3& point, const Slope& slope);
    NVCC_HD LineSegment(const Point3& point, const Point3& otherPoint);

    NVCC_HD inline const Point3& getStartPoint(void) const {return point;}
    NVCC_HD inline const Point3& getEndPoint(void) const {return endPoint;}
    NVCC_HD inline const Point3& getOppositeEndpoint(const Point3& p) const {
    #ifdef DEBUG
        assert(p == endPoint || p == point);
    #endif
        return (p == endPoint ? point : endPoint);
    }

    //return true iff point in on this segment, endpoint inclusive
    NVCC_HD bool isOnSegment(const Point3& testPoint) const;

    NVCC_HD bool doesSegmentIntersect(const LineSegment& other) const;
        
    //get the distance on the segments (endpoints inclusive)
    NVCC_HD double minSeperationDistance(const LineSegment& other) const;

    //get the distance point to segment (endpoints inclusive)
    NVCC_HD double minSeperationDistance(const Point3& other) const;

    //return the length of this segment
    NVCC_HD inline double length(void) const {return getPointDistance(point, endPoint);}
};

std::ostream &operator<<(std::ostream& os, const LineSegment& seg);