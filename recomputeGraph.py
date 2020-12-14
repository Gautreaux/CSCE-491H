# TODO - title description and whatever

# TODO - abandoned?

from recomputeFramework import GenericRecompute, NoValidRedefinedPath
from gcodeParser import ParseResult

NOZZLE_SEPERATION_DIST_MM = 25

class GraphRecompute(GenericRecompute):
    def __init__(self):
        pass

    def __str__(self):
        return "Graph Based Recompute"

    def recomputePath(self, pathRepr):
        if type(pathRepr) != ParseResult:
            raise TypeError("type of pathRepr should be ParseResult")

        layerTimes = []

        print(f"Starting: {self}")

        for layerGenerator in pathRepr.layerGeneratorGenerator():
            layerList = list(layerGenerator)
            layerList = list(filter(lambda x: x.isPrint(), layerList))

            # compute layer meta
            
            # finding the layer limits
            minX = min(map(lambda x: min(x.point[0], x.pointTwo[0]), layerList))
            maxX = max(map(lambda x: max(x.point[0], x.pointTwo[0]), layerList))
            minY = min(map(lambda x: min(x.point[1], x.pointTwo[1]), layerList))
            maxY = max(map(lambda x: max(x.point[1], x.pointTwo[1]), layerList))

            print(f"Layer limits: minX {minX}, maxX {maxX}, minY {minY}, maxY {maxY}")
            
            minLen = min(map(lambda x: x.printAmt, layerList))
            maxLen = max(map(lambda x: x.printAmt, layerList))

            print(f"Path lens: min {minLen}, max {maxLen}, rat: {maxLen / minLen}")

            if maxLen > NOZZLE_SEPERATION_DIST_MM:
                print(f"Max path len {maxLen} larger than min seperation {NOZZLE_SEPERATION_DIST_MM}, should subdivide")
           
            from layerDisplay import displayAndIterateLayer
            displayAndIterateLayer(layerList, NOZZLE_SEPERATION_DIST_MM)
            # setting up some things
            printedSegments = [False]*len(layerList)

            a1Pos = None
            a2Pos = None

            while False in printedSegments:
                if a1Pos == None and a2Pos == None:
                    # this is the first one, so we need to pick spots

                    # the pick criteria is to place one agent on the left most point
                    # and the other approximately 110% of min seperation away

                    startIndex = None
                    for i in range(len(layerList)):
                        if layerList[i].point[0] == minX:
                            printedSegments[i] = True
                            a1Pos = layerList[i].pointTwo
                            startIndex = i
                            break
                        if layerList[i].pointTwo[0] == minX:
                            printedSegments[i] = True
                            a1Pos = layerList[i].point
                            startIndex = i
                            break

                    if startIndex == None:
                        raise ValueError("Could not find matching start in graph recompute")

                    # find the segment who is closest to A1's seg but still far enough
                    sepDistances = list(map(lambda x: layerList[i].minSeperation(x), layerList))
                    assert(min(sepDistances) == 0.0)

                    

                    print(min(sepDistances))



                    


        return sum(layerTimes)