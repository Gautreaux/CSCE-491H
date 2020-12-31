#include "LineSegment.h"

#include <algorithm> // std::min

LineSegment::LineSegment(const Point3& point, const Point3& otherPoint) :
    Line(point, otherPoint), endPoint(otherPoint)
{
}

std::ostream &operator<<(std::ostream& os, const LineSegment& seg){
    os << seg.getStartPoint() << " -> " << seg.getEndPoint();
    return os;
}

bool LineSegment::isOnSegment(const Point3& testPoint) const {
    if(!isOnLine(testPoint)){
        //the point is not on the base line, so it cannot be on the segment
        return false;
    }

    //need to check if this point is one of the end points
    if(point == testPoint || endPoint == testPoint){
        return true;
    }

    // to explain this magic
    //  we compute the x-dimension of the vector from testPoint to either endPoint
    //  we then xor the parity (by multiplication)
    //  we expect the parity to be odd if these derived vectors are pointed opposite ways
    //      if the vectors point the same way, the point is outside the limits
    // this could use the full vectors, but no need for the overhead
    return ((point.getX() - testPoint.getX()) * (endPoint.getX() - testPoint.getX())) < 0;
}

double LineSegment::minEndPointPairDistance(const LineSegment& other) const {
    return  std::min(
                std::min(getPointDistance(point, other.point),
                         getPointDistance(endPoint, other.point)),
                std::min(getPointDistance(point, other.endPoint),
                         getPointDistance(endPoint, other.endPoint))
            );
}

bool LineSegment::doesSegmentIntersect(const LineSegment& other) const {
    if(isCollinear(other)){
        // only need to see if the endpoints overlap
        return (isOnSegment(other.point) || isOnSegment(other.endPoint));
    }
    if(isParallel(other)){
        return false;
    }

    try{
        Point3 intersection = getLineIntersectPoint(other);
        return (isOnSegment(intersection) && other.isOnSegment(intersection));
    } catch (const Line::NoIntersectionException& e){
        return false;
    } //let a collinear collision exception pass through ok
}

//TODO - adapted from python code, could be much cleaner
double LineSegment::minSeperationDistance(const LineSegment& other) const {
    if(doesSegmentIntersect(other)){
        return 0;
    }

    if(isCollinear(other)){
        //the segments dont interset, simply find the closest two endpoints
        return minEndPointPairDistance(other);
    }else if(isParallel(other)){
        // I think this logic is the same as the main logic:
        //  min is either between endpoint pairs or some endpoint projected onto this segment
        //  in the original python version, there was an optimization w.r.t. the projections
        //      but I am no longer convinced that it will work
        // TODO ^ evaluate above statement
    }

    //depending on the way the lines approach, the endpoints may be the closest
    double minDist = minEndPointPairDistance(other);
    if(minDist == 0){
        // these segments share an endpoint
        return 0;
    }

    //or it may be a projection of an endpoint onto the segments
    Point3 projectedPoint(0,0,0);
    double tempDist;

    projectedPoint = other.getProjectionPoint(this->point);
    if(other.isOnSegment(projectedPoint)){
        minDist = std::min(minDist, getPointDistance(projectedPoint, this->point));
    }

    projectedPoint = other.getProjectionPoint(this->endPoint);
    if(other.isOnSegment(projectedPoint)){
        minDist = std::min(minDist, getPointDistance(projectedPoint, this->endPoint));
    }

    projectedPoint = this->getProjectionPoint(other.point);
    if(this->isOnSegment(projectedPoint)){
        minDist = std::min(minDist, getPointDistance(projectedPoint, other.point));
    }

    projectedPoint = this->getProjectionPoint(other.endPoint);
    if(this->isOnSegment(projectedPoint)){
        minDist = std::min(minDist, getPointDistance(projectedPoint, other.endPoint));
    }

    return minDist;
}