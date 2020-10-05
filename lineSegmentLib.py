# file for line segment repr and associated functions:
#this is certainly not the most efficient line segment library

#intended and tested for 3d lines

from lineLib import Line
from vectorLib import *

#TODO - can these be made more efficenet by ordering the points 
class LineSegment(Line):
    #always assumed endpoint inclusive
    def __init__(self, pointA, pointB):
        super().__init__(pointA, pointTwo=pointB)
        self.pointTwo=pointB

    #return true iff the point is on the segment endpoint inclusive
    def isOnSegment(self, point):
        if self.isOnLine(point) is False:
            return False
        #make sure it is on line within the endpoints
        if point == self.point or point == self.pointTwo:
            return True
        
        #check that the vector from one point to the end 
        return (
            unitize(vectorSub(point, self.point)) ==
            unitize(reverseVector(vectorSub(point, self.pointTwo)))
        )

    # get the length of this segment
    # TODO - needs unit testing
    def length(self):
        return pointDistance(self.point, self.pointTwo)

    #return the minimum distance between endpoints
    def minEndpointsDistance(self, other):
        return min(
                    pointDistance(self.point, other.point),
                    pointDistance(self.point, other.pointTwo),
                    pointDistance(self.pointTwo, other.point),
                    pointDistance(self.pointTwo, other.pointTwo)
                )

    #minimum seperation, bounded within the line segments
    #TODO - this should be tested more before moving to 3d
    def minSeperation(self, other):
        # this is a magic function that was taken from 
        #   http://geomalgorithms.com/a07-_distance.html#dist3D_Segment_to_Segment()

        #TODO - there are several redundant checks in here

        #if collinear
        if self.isCollinear(other):
            #see if the endpoints overlap
            if (
                    self.isOnSegment(other.point) or
                    self.isOnSegment(other.pointTwo) or
                    other.isOnSegment(self.point) or 
                    other.isOnSegment(self.pointTwo)
            ):
                #collinear and overlapping endpoints
                return 0
            return self.minEndpointsDistance(other)

        #if parallel
        if self.isParallel(other):

            if(
                self.isOnSegment(self.getProjectionPoint(other.point)) or
                self.isOnSegment(self.getProjectionPoint(other.pointTwo))
            ):
                return super().minSeperation(other)
            return self.minEndpointsDistance(other)

        #if it works...
        otherProjection1 = other.getProjectionPoint(self.point)
        otherProjection2 = other.getProjectionPoint(self.pointTwo)
        
        selfProjection1 = self.getProjectionPoint(other.point)
        selfProjection2 = self.getProjectionPoint(other.pointTwo)

        minPoint = self.minEndpointsDistance(other)

        distList = (minPoint,
                    pointDistance(otherProjection1, self.point) if other.isOnSegment(otherProjection1) else float('inf'),
                    pointDistance(otherProjection2, self.pointTwo) if other.isOnSegment(otherProjection2) else float('inf'),
                    pointDistance(selfProjection1, other.point) if other.isOnSegment(selfProjection1) else float('inf'),
                    pointDistance(selfProjection2, other.pointTwo) if other.isOnSegment(selfProjection2) else float('inf')
        )

        return min(distList)
    
    def reverse(self):
        t = self.point
        self.point = self.pointTwo
        self.pointTwo = t
        # by line lib standard, vector always points non-negative x

    # return a list of segments such that
    #   each segment is max_len (except for the last one, which is shorter)
    def splitToLen(self, max_len=float('inf')):
        if self.length() > max_len:
            v = getUnitVector(self.point, self.pointTwo)
            
            #TODO - this should become vectorMult once that is checked for accuracy
            newPoint = tuple(map(lambda x,y: x*max_len+y, v, self.point))

            toReturn = [LineSegment(self.point, newPoint)]

            other = LineSegment(newPoint, self.pointTwo)

            for seg in other.splitToLen(max_len):
                toReturn.append(seg)

            return toReturn
        else:
            return [self]

if __name__ == "__main__":
    seg1 = LineSegment((0,0,0), (1,1,1))
    seg2 = LineSegment((1,1,1), (2,2,2))
    seg3 = LineSegment((1,0,0), (2,1,1))

    assert(seg1.isParallel(seg2))
    assert(seg1.isParallel(seg3))

    print("Segment parallel passed")

    assert(seg1.isCollinear(seg2))
    assert(not seg1.isCollinear(seg3))

    print("Segment collinear passed")

    p1 = (.5,.5,.5)
    p2 = (1,1,1)
    p3 = (-1,-1,-1)
    p4 = (2,2,2)

    assert(seg1.isOnSegment(p1))
    assert(seg1.isOnSegment(p2))
    assert(not seg1.isOnSegment(p3))
    assert(not seg1.isOnSegment(p4))
    assert(not seg2.isOnSegment(p1))
    assert(not seg3.isOnLine(p3))

    print("All on segment checks passed")

    segMin1 = LineSegment((0,0,0), (1,0,0))
    segMin2 = LineSegment((0,0,1), (1,0,1))
    segMin3 = LineSegment((0,0,0), (-1,-6,-8))

    assert(segMin1.minSeperation(segMin2) == 1)
    assert(segMin1.minSeperation(segMin3) == 0)

    # TODO - tons of unit test

    print("All min distance check passed")

    print("All segment checks passed")





# def minSeperation(self, other):
#         # this is a magic function that was taken from 
#         #   http://geomalgorithms.com/a07-_distance.html#dist3D_Segment_to_Segment()

#         vecU = vectorSub(self.pointTwo, self.point)
#         vecV = vectorSub(other.pointTwo, other.point)
#         vecW = vectorSub(self.point, other.point)

#         floatA = vectorDot(vecU, vecU)
#         floatB = vectorDot(vecU, vecV)
#         floatC = vectorDot(vecV, vecV)
#         floatD = vectorDot(vecU, vecW)
#         floatE = vectorDot(vecV, vecW)
#         floatDD = floatA*floatC-floatC*floatC

#         sc = floatDD
#         sN = floatDD
#         sD = floatDD
#         tc = floatDD
#         tN = floatDD
#         tD = floatDD

#         if self.isParallel(other):
#             sN = 0
#             sD = 1.0
#             tN = floatE
#             tD = floatC
#         else:
#             sN = floatB*floatE - floatC*floatD
#             tN = floatA*floatE - floatB*floatD

#             if sN < 0:
#                 sN = 0
#                 tN = floatE
#                 tD = floatC
#             elif sN > sD:
#                 sN = sD
#                 tN = floatE + floatB
#                 tD = floatC
        
#         if (tN < 0):
#             tN = 0
#             if -floatD < 0:
#                 sN = 0
#             elif -floatD < floatA:
#                 sN = sD
#             else:
#                 sN = -floatD
#                 sD = floatA
#         elif (tN > tD):
#             tN = tD
#             if (-floatD + floatB) < 0:
#                 sN = 0
#             elif (-floatD + floatB) > floatA:
#                 sN = sD
#             else:
#                 sN = -floatD + floatB
#                 sD = floatA
        
#         sc = 0 if round(sN, 8) == 0 else sN/sD
#         tc = 0 if round(tN, 8) == 0 else tN/tD

#         dp = vectorAdd(vecW, vectorSub(
#             vectorMult(vecU, sc), vectorMult(vecV, tc)
#         ))

#         return vectorMagnitude(dp)