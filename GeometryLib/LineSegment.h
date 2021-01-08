#pragma once

#include "../pch.h"

#include "Line.h"
#include "Point3.h"

class LineSegment : public Line{
protected:
    Point3 endPoint;

    //return the distance between the enspoints between segments that are closets
    double minEndPointPairDistance(const LineSegment& other) const;
public:
    //LineSegment(const Point3& point, const Slope& slope);
    LineSegment(const Point3& point, const Point3& otherPoint);

    inline const Point3& getStartPoint(void) const {return point;}
    inline const Point3& getEndPoint(void) const {return endPoint;}
    inline const Point3& getOppositeEndpoint(const Point3& p) const {
    #ifdef DEBUG
        assert(p == endPoint || p == point);
    #endif
        return (p == endPoint ? point : endPoint);
    }

    //return true iff point in on this segment, endpoint inclusive
    bool isOnSegment(const Point3& testPoint) const;

    bool doesSegmentIntersect(const LineSegment& other) const;
        
    //get the distance on the segments (endpoints inclusive)
    double minSeperationDistance(const LineSegment& other) const;

    //get the distance point to segment (endpoints inclusive)
    double minSeperationDistance(const Point3& other) const;
};

std::ostream &operator<<(std::ostream& os, const LineSegment& seg);