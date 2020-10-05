#class for a singular gcode segment

from lineSegmentLib import LineSegment


class gcodeSegment(LineSegment):
    def __init__(self, start, end, printAmt = None):
        #self.startPos = self.point
        #self.endPos = self.pointTwo
        super().__init__(start, end)
        self.printAmt = printAmt

    def isPrint(self):
        return not(self.printAmt == None)

    # return true iff this segment is fully in the z plane
    def onZ(self, z):
        return self.point[2] == z and self.pointTwo[2] == z

    # return the start and end points as ((x0,y0),(x1,y1))
    def asXYPairPair(self):
        return ((self.point[0], self.point[1]),(self.pointTwo[0], self.pointTwo[1]))

    # return the start and end points as ((x0,x1),(y0,y1))
    def asXYPlotPair(self):
        return ((self.point[0], self.pointTwo[0]),(self.point[1], self.pointTwo[1]))

    def __str__(self):
        return f"{self.point}->{self.pointTwo}{'P' if self.isPrint() else 'M'}"

if __name__ == "__main__":
    seg1 = gcodeSegment((0,0,0), (1,1,1))
    seg2 = gcodeSegment((1,1,1), (2,2,2))
    seg3 = gcodeSegment((4,4,4), (8,8,8))
    seg4 = gcodeSegment((2,2,2), (0,0,0))

    assert(seg1.isXYParallel(seg2))
    assert(seg1.isXYParallel(seg3))
    assert(seg1.isXYParallel(seg4))

    print("All test assertions passed.")