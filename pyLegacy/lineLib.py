# file for line repr and associated functions:
#this is certainly not the most efficient line library

#intended and tested for 3d lines

from vectorLib import *

#an infinitely long line
class Line(object):
    def __init__(self, point, vector=None, pointTwo=None):
        if vector is None and pointTwo is None:
            raise ValueError("Either a or secondary point must be provided")
        if vector is not None and pointTwo is not None:
            raise ValueError("Cannot provide both a vector and a secondary point")

        self.point = point
        if vector is not None:
            self.vector = unitize(vector)
        else:
            self.vector = getUnitVector(point, pointTwo)
        
        if self.vector[0] < 0:
            self.vector = reverseVector(self.vector)

    #true iff self is parallel to other
    def isParallel(self, other):
        return (self.vector == other.vector or
                reverseVector(self.vector) == other.vector)
        #TODO- can remove the reverse?
        #   orientation is enforced in constructor

    #true iff self is collinear to other
    def isCollinear(self,other):
        if not self.isParallel(other):
            return False
        sepVec = getUnitVector(self.point, other.point)

        if sum(sepVec) == 0:
            #same point and parallel
            return True
        #TODO - this is inefficient
        return self.isParallel(Line(self.point, sepVec))
    
    def isSkew(self, other):
        raise NotImplementedError()

    #minimum distance between the two lines
    def minSeperation(self, other):
        if self.isCollinear(other):
            return 0

        points = vectorSub(other.point, self.point)
        if self.isParallel(other):
            
            return vectorMagnitude(vectorCross(self.vector, points))

        crossVec = vectorCross(self.vector, other.vector)
        crossMag = vectorMagnitude(crossVec)

        return (abs(vectorDot(crossVec, points))/crossMag)

    #is the point on the line
    def isOnLine(self, point):
        return self.isCollinear(Line(point, self.vector))

    #return the intersection of this line with other
    #   returns point if singular intersection
    #   inf if collinear
    #   none if parallel/skew 
    def getIntersectionPoint(self, other):
        if self.isCollinear(other):
            return float('inf')
        
        if self.isSkew(other) or self.isParallel(other):
            return None

        raise NotImplementedError()

    # return the point on self that is closest to other
    #   for parallel/collinear lines will be self.point
    def getClosestApproachPoint(self, other):
        if self.isParallel(other): # collinearity implies parallel
            return self.point
        
        raise NotImplementedError()

    # return the point on self that point projects to
    def getProjectionPoint(self, point):
        vectorAP = vectorSub(point, self.point)
        vectorAB = vectorSub(self.pointTwo, self.point)

        vectorValue = vectorDot(vectorAP,vectorAB)/vectorDot(vectorAB, vectorAB)
        return vectorAdd(self.point, vectorMult(vectorAB, vectorValue))

    # return the distance from a point to a line
    def getProjectionDistance(self, point):
        raise NotImplementedError()


if __name__ == "__main__":
    line1 = Line((0,0,0), vector=(1,1,1))
    line2 = Line((0,0,0), pointTwo=(1,1,1))
    line3 = Line((4,4,4), pointTwo=(8,8,8))
    line4 = Line((1,0,0), vector=(1,1,1))
    line5 = Line((1,0,0), pointTwo=(7,6,6))
    line6 = Line((2,2,2), pointTwo=(0,0,0))
    line7 = Line((1,2,3), vector=(6,7,9))

    assert(line1.isParallel(line2))
    assert(line1.isParallel(line3))
    assert(line1.isParallel(line4))
    assert(line1.isParallel(line5))
    assert(line1.isParallel(line6))
    assert(not line1.isParallel(line7))
    
    print("Line parallel checks passed")
    
    assert(line1.isCollinear(line2))
    assert(line1.isCollinear(line6))
    assert(not line1.isCollinear(line4))
    assert(line4.isCollinear(line5))
    assert(not line1.isCollinear(line7))

    print("Line collinear checks passed")

    assert(line1.minSeperation(line2) == 0)
    assert(round(line1.minSeperation(line4), 4) == .8165)
    assert(round(line1.minSeperation(line7), 4) == .2673)

    print("Line distance checks passed")

    p1 = (2,2,2)
    assert(line1.isOnLine(p1))
    assert(line2.isOnLine(p1))
    assert(not line7.isOnLine(p1))

    print("Line onLine checks passed")

    print("All line check passed")
