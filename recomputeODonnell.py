#implements variants of the ODonnell method
# https://people.csail.mit.edu/tlp/publications/coordination-o-donnell.pdf

#generic idea:
# split the provided path into two paths
# assign one path per agent
# place the paths in a 2d grid
# determine where these paths coincide
# find a shortest path that covers all segments


from abc import abstractmethod
from functools import total_ordering

from aStar import *
from gcodeParser import ParseResult
from lineLib import Line
from recomputeFramework import GenericRecompute, NoValidRedefinedPath

class ODonnellParamenters(object):  
    MIN_SEPERATION_MM = 25
    COLLIDING = 0.0
    NON_COLLIDING = 1.0
    NON_EVALUATED = .5

    # probably a more formal way to do this:
    def __init__(self):
        raise NotImplementedError("This class is 'static'")


class GenericODonnell(GenericRecompute):
    def __init__(self):
        pass

    def __str__(self):
        "Generic ODonnell implementation"

    # convert a layerIterable into two paths
    #   these sub paths should be represented as 
    @abstractmethod
    def createPaths(self, layerIterable):
        pass

    # do the grid search and return the time taken
    # the grid search should lazily evaluate the collision map,
    #   but the function computerCollisions() is provided to brute force
    #   if necessary
    @abstractmethod
    def doGridSearch(self, layerGrid):
        pass

    def buildCollisionGrid(self, pathA, pathB):
        width = len(pathA)
        height = len(pathB)

        layerGrid = [None]*height

        for i in range(height):
            layerGrid[i] = [ODonnellParamenters.NON_EVALUATED]*width
        
        return layerGrid

    #probably want to override this function in further implementations
    def areSegmentsColliding(self, segmentA, segmentB) -> bool:
        return segmentA.minSeperation(segmentB) < ODonnellParamenters.MIN_SEPERATION_MM

    def computeCollisions(self, pathA, pathB, layerGrid):
        for i in range(len(pathA)):
            for j in range(len(pathB)):
                if not self.areSegmentsColliding(pathA[i], pathB[j]):
                    layerGrid[i][j] = ODonnellParamenters.NON_COLLIDING
                else:
                    layerGrid[i][j] = ODonnellParamenters.COLLIDING

    #path repr should be a ParseResult
    def recomputePath(self, pathRepr):
        if type(pathRepr) != ParseResult:
            raise TypeError("type of pathRepr should be ParseResult")

        layerTimes = []

        print(f"Starting: {self}")

        for layerGenerator in pathRepr.layerGeneratorGenerator():
            # print("\tStarting creating paths...")

            # from  layerDisplay import displayAndIterateLayer
            # layerGenerator = displayAndIterateLayer(layerGenerator, ODonnellParamenters.MIN_SEPERATION_MM/2)

            (pathA, pathB) = self.createPaths(layerGenerator)

            #DEBUG - something is wrong in here
            if(len(pathA)*len(pathB) == 0):
                print("Detected invalid state: subpath has 0 length")
                continue

            # print("\tFinished. Starting path search...")
            
            layerGrid = self.buildCollisionGrid(pathA, pathB)


            t = self.doGridSearch(layerGrid)

            if t == 0:
                def tempGen():
                    for seg in pathA:
                        yield seg
                
                from layerDisplay import displayAndIterateLayer
                displayAndIterateLayer(tempGen(), ODonnellParamenters.MIN_SEPERATION_MM/2)
                pass

            layerTimes.append(t)

            # print("For debugging, breaking loop after one layer.")
            # break

        return sum(layerTimes)


#basic idea:
# repr the segments on a 2d grid,
#   the x axis is agent X is forward pass a specified by file
#   the y axis is agent Y is reverse pass
# thus, x[0] is the first movement
#   y[0] the last
# the cell (x,y) is blocked if the agents can collide in any configuration
#   while traversing their subsegments
# This will cause all cells to be blocked where x+y = total segments
# Goal: find shortest path to this blocked chain from 0,0 or maxX,maxY
#   this will be (optimally) along the 45 degree angle
class InwardODonnell(GenericODonnell):

    def __str__(self):
        return "Inward ODonnell Implementation"

    def createPaths(self, layerIterable):
        l = list(layerIterable)
        lR = l.copy()
        lR.reverse()

        # needed for graph search phase
        self.maxPathLen = sum(map(lambda x:x.length(), l))

        self.l = l
        self.lR = lR

        return (l, lR)

    def areSegmentsColliding(self, x, y):
        b = super().areSegmentsColliding(self.l[x], self.lR[y])
        self.grid[x][y] = (ODonnellParamenters.COLLIDING if b else ODonnellParamenters.NON_COLLIDING)
        return b
    
    def doGridSearch(self, layerGrid, displayGrid=False):
        # the state representation shall be:
        #   pos=(x,y)
        #   parent=(a,b)
        #   dist=d
        @total_ordering
        class SearchState(object):
            MAX_LEN = float('inf')

            def __init__(self, x, y, above, parentState=None, dist=0, length=0):
                self.parent = parentState
                self.x = x
                self.y = y
                self.above = above # helps with state generation
                self.dist = dist # total dist elapsed
                self.length = length # singular-dist elapsed
            
            def priority(self):
                return (self.MAX_LEN - self.dist) + self.length
            
            def __eq__(self, other):
                if type(other) == type(None):
                    return False
                return self.priority() == other.priority()
            
            def __ne__(self, other):
                if type(other) == type(None):
                    return True
                return self.priority() != other.priority()
            
            def __le__(self, other):
                if type(other) == type(None):
                    return False
                return self.priority() < other.priority()

        SearchState.MAX_LEN = self.maxPathLen

        gridLen = len(layerGrid)
        self.grid = layerGrid

        visitedDict = {}
        def isVisited(state):
            posX = state.x
            posY = state.y
            if posX not in visitedDict:
                visitedDict[posX] = set()
            if posY in visitedDict[posX]:
                return True
            visitedDict[posX].add(posY)
            return False
        
        def isGoal(state):
            # TODO - why does this assertion fail?
            # TODO - this should be x+y == gridLen?
            # assert((state.x + state.y >= gridLen) == (state.dist >= self.maxPathLen))
            return (state.x + state.y == gridLen)

        def stateGenerator(state):
            if state is None:
                #starting states
                # return [SearchState(gridLen-1, gridLen-1, False)]
                return [SearchState(0,0, True), SearchState(gridLen-1, gridLen-1, False)]
            returnStates = []
            x = state.x
            y = state.y
            d = state.dist
            l = state.length

            if state.above is True:
                # go right
                segX = self.l[x]
                returnStates.append(SearchState(x+1, y, True, state, d + segX.length(), l + segX.length()))
                # go down
                segY = self.lR[y]
                returnStates.append(SearchState(x, y+1, True, state, d + segY.length(), l + segY.length()))
                # go down-right
                if self.areSegmentsColliding(x,y) is False:
                    seg = max([segX, segY], key=(lambda x: x.length()))
                    returnStates.append(SearchState(x+1,y+1, True, state,
                            d+segX.length()+segY.length(), l+seg.length()))
            else:
                # go left
                segX = self.l[x-1]
                returnStates.append(SearchState(x-1, y, False, state, d+segX.length(), l+segX.length()))
                # go up
                segY = self.lR[y-1]
                returnStates.append(SearchState(x, y-1, False, state, d+segY.length(), l+segY.length()))
                # go down-right
                if self.areSegmentsColliding(x,y) is False:
                    seg = max([segX, segY], key=(lambda x: x.length()))
                    returnStates.append(SearchState(x-1,y-1, False, state,
                            d+segX.length()+segY.length(), l+seg.length()))
            return returnStates

        try:
            finalState = aStar(stateGenerator, isGoal, isVisited)
        except NoValidPath:
            raise NoValidRedefinedPath()

        if displayGrid is True or finalState.length == 0:
            from odonnellGridDisplay import displayGrid
            # todo - should take a generator from the state to impose the path
            displayGrid(layerGrid)

        # debug code
        if finalState.length == 0:
            print(f"Error, zero finalState length")

        print(f"Final length was {finalState.length}, orig {self.maxPathLen}, {round(finalState.length/self.maxPathLen*100, 6)}%")

        return finalState.length

class InwardOdonnellSplit(InwardODonnell):

    def __str__(self):
        return "Inward Split Odonnell"

    def createPaths(self, layerIterable):
        rawPath = list(layerIterable)

        lastXY = rawPath[0].point

        pathA = []
        for segment in rawPath:
            #check that a reverse is not needed
            if segment.point != lastXY:
                segment.reverse()

            subSeg = segment.splitToLen(ODonnellParamenters.MIN_SEPERATION_MM)

            #TODO - remove debugging
            tmp = sum(map(lambda x:x.length(), subSeg))
            assert(max(map(lambda x: x.length(), subSeg)) <= ODonnellParamenters.MIN_SEPERATION_MM + .0001)
            assert(abs(tmp - segment.length()) < .0001)

            for sub in subSeg:
                pathA.append(sub)

        pathB = pathA.copy()
        pathB.reverse()

        # TODO - remove debugging
        assert(max(map(lambda x: x.length(), pathA)) <= ODonnellParamenters.MIN_SEPERATION_MM + .0001)

        # TODO - this **should** be equal to len of pathA and pathB
        t = sum(map(lambda x:x.length(), rawPath))
        tt = sum(map(lambda x:x.length(), pathA))
        ttt = sum(map(lambda x:x.length(), pathB))
        assert(abs(t-tt) < .0001 and abs(t-ttt) < .0001)
        self.maxPathLen = t
        self.l = pathA
        self.lR = pathB

        return (pathA, pathB)


# TODO - halvsies odonnell
#   more complicated than it should be thanks to state class being
#       inside the state