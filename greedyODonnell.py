# This file is archive

#implement (a variant of) the ODonnell coordination method
# https://people.csail.mit.edu/tlp/publications/coordination-o-donnell.pdf

#basic idea:
# repr the segments on a 2d grid,
#   the x axis is agent X is forward pass a specified by file
#   the y axis is agent Y is reverse pass
# thus, x[0] is the first movement
#   y[0] the last
# the cell (x,y) is blocked if the agents can collide in any configuration
#   while traversing their subsegments
# This will cause all cells to be blocked where x+y = total segments
# Goal: find shortest path to this blocked chain from 0,0
#   this will be (optimally) along the 45 degree angle

from gcodeParser import ParseResult

import matplotlib.pyplot as plt
from matplotlib import cm
import numpy as np

MIN_SEPERATION_MM = 25

#TODO - refactor
class grid2DSquare():
    def __init__(self, size, defaultValue=None):
        size += 1
        self.data = [None]*size
        for i in range(size):
            self.data[i] = [defaultValue]*size
        
        #there is a chance that the start and end overlap
        # so we add another row 'below' the bottom where 
        # all cells are passable
        #i.e. one agent does the whole thing
        self.data[-1] = [0.0]*size

    def getValue(self, x, y):
        return self.data[x][y]
    
    def setValue(self, x, y, v):
        self.data[x][y] = v

    def display(self):
        plt.imshow(np.array(self.data), interpolation='nearest')
        plt.colorbar()
        plt.title("0.0 is clear, 1.0 is collision, 8249-.35 gcode")
        plt.show()
        raise NotImplementedError()


def buildGridRepr(segList):
    segCount = len(segList)
    # grid = grid2DSquare(segCount, GridTypes.COLLISION)
    grid = grid2DSquare(segCount, 1.0)

    plt.figure()    

    totalClear = 0
    #compute the collisions for each segment
    for x in range(segCount):
        for y in range(segCount):
            xSeg = segList[x]
            ySeg = segList[-y-1]

            if not collidingSegments(xSeg, ySeg, MIN_SEPERATION_MM):
                # grid.setValue(x,y,GridTypes.CLEAR)
                grid.setValue(x,y,0.0)
                totalClear += 1
    
    print(f"In {segCount**2} pairs, {totalClear} are collision free")

    grid.display()

    # TODO - some form of search and traversal

    return grid


#return true iff the two segments collide
def collidingSegments(aSeg, bSeg, minSeperationMM):
    #collision conditions
    #   xDim of aSeg is > xDim of bSeg (any portion)
    #   distance between segments < minSeperationMM

    if (max(aSeg.point[0], aSeg.pointTwo[0]) >
            min(bSeg.pointTwo[0], bSeg.point[0])):
        return True

    if(aSeg.minSeperation(bSeg) < minSeperationMM):
        return True

    return False

if __name__ == "__main__":
    from gcodeParser import *

    parser = gcodePathParser()
    parseResult = parser.parseFile("gcodeFullSet/8-gcode/8249.gcode")

    parseResult.getReport()
    zLayers = list(parseResult.getZLayers())

    print(f"Z layers: {zLayers}")

    # technically the z layers should be in sorted order and 
    #   this can become much more efficient
    # TODO - change to layer generator generator framework
    oneZ = [f for f in parseResult if f.onZ(zLayers[0])]
    print(f"Z layer {zLayers[0]} contains {len(oneZ)} segments")
    buildGridRepr(oneZ)