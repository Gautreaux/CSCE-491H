#include "Line.cuh"

Line::Line(const Point3& point, const Slope& slope) : point(point), slope(slope){};

Line::Line(const Point3& point, const Point3& otherPoint) :
    point(point), slope(point, otherPoint)
{
    //TODO - do this one too
    // is this done though?
}

bool Line::isParallel(const Line& other) const {
    return slope == other.slope || slope == other.slope.reverse();
}

bool Line::isCollinear(const Line& other) const {
    if(!isParallel(other)){
        return false;
    }

    Line spanLine(this->point, other.point);
    return isParallel(spanLine);
}

bool Line::isOnLine(const Point3& testPoint) const {
    return isCollinear(Line(testPoint, this->slope));
}

Point3 Line::getLineIntersectPoint(const Line& other) const {
    if(isCollinear(other)){
#ifndef __CUDACC__
        throw CollinearIntersectionException();
#else
        return Point3Any();
#endif
    }
    //if parallel/skew, throw no intersecton?
    //  nah, part below should catch it

    // easiest check
    if(point == other.point){
        return point;
    }
    if(other.isOnLine(this->point)){
        return point;
    }
    if(this->isOnLine(other.point)){
        return other.point;
    }

    //magic: https://math.stackexchange.com/questions/270767/find-intersection-of-two-3d-lines/271366

    Vector3 pointsVector(other.point - this->point, false);

    double cross1Mag = cross_product(other.slope, pointsVector).getMagnitude();
    double cross2Mag = cross_product(other.slope, this->slope).getMagnitude();
    if(DOUBLE_EQUAL(cross1Mag, 0) || DOUBLE_EQUAL(cross2Mag, 0)){
#ifndef __CUDACC__
        throw NoIntersectionException();
#else
        return Point3Any();
#endif
    }

    Vector3 offsetVector = this->slope * (cross1Mag/cross2Mag);

    Point3 p = this->point + offsetVector;
    if(other.isOnLine(p)){
        return p;
    }else{
        p = this->point - offsetVector;
#ifdef DEBUG
        if(other.isOnLine(p) == false){
            printf("Assertion Failure inbound\n");
            std::cout << "Point p: " << p << ", Line (other): " << other << std::endl;
            std::cout << "Line (this): " << *this << ", offsetVector: " << offsetVector << std::endl;
            std::cout << "Cross1Mag: " << cross1Mag << ", cross2Mag: " << cross2Mag << std::endl;
        }
#endif
#ifdef NVCC_DEBUG
#ifdef __CUDACC__
        if(other.isOnLine(p) == false){
            unsigned int idx = (uint64_t)(this) % (1 << 30);
            printf("(%d) Assertion Failure inbound:\n", idx);
            printf("(%d) Point p : (%f, %f, %f)\n", idx, p.getX(), p.getY(), p.getZ());
            printf("(%d) Line (this): [(%f, %f, %f) (%f, %f, %f)]\n", idx,
                this->getPoint().getX(), this->getPoint().getY(), this->getPoint().getZ(),
                this->getSlope().getX(), this->getSlope().getY(), this->getSlope().getZ());
            printf("(%d) Line (other): [(%f, %f, %f) (%f, %f, %f)]\n", idx,
                other.getPoint().getX(), other.getPoint().getY(), other.getPoint().getZ(),
                other.getSlope().getX(), other.getSlope().getY(), other.getSlope().getZ());
            printf("(%d) Offset Vector: (%f, %f, %f)\n", idx,
                offsetVector.getX(), offsetVector.getY(), offsetVector.getZ());
            printf("(%d) Cross 1 Mag: %f, Cross 2 Mag: %f\n", idx, cross1Mag, cross2Mag);
        }
#endif
#endif
        assert(other.isOnLine(p));
        return p;
    }

}

Point3 Line::getProjectionPoint(const Point3& testPoint) const {
    Vector3 ap(testPoint - point, false);
    
    double projectionMagnitude = dot_product(ap, slope)/dot_product(slope, slope);
    return (point) + (projectionMagnitude * slope);
}

std::ostream& operator<<(std::ostream& os, const Line& line){
    os << "[" << line.getPoint() << " " << line.getSlope() << "]";
    return os;
}