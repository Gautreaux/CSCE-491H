from lineSegmentLib import LineSegment
from typing import Any, List, Set, Tuple, Union
from vectorLib import pointDistance
from recomputeFramework import GenericRecompute


from bitmapLib import BitMap
from gcodeParser import ParseResult
from gcodeSegment import gcodeSegment
from priorityQueueLib import PriorityQueue

# TODO - this is hacky and should be refactored
from iterativeAStarSimple import simpleIterativeAStar

MIN_SEPERATION_MM = 25

class IndexableStateGenerator():
    def __init__(self, generator, layerList) -> None:
        raise NotImplementedError()

    def next(self) -> Union[None, 'SearchState']:
        raise NotImplementedError()

    def getPriority(self) -> int:
        raise NotImplementedError()


class SearchState():
    # TODO  - Typing on statesCache
    def __init__(self, layerList : List[gcodeSegment], statesCache : Any) -> None:
        raise NotImplementedError()

    def isGoal(self) -> bool:
        raise NotImplementedError()

    def getSuccessorGenerator(self) -> IndexableStateGenerator:
        raise NotImplementedError()

    def heuristic(self) -> IndexError:
        raise NotImplementedError()

class IterativeAStar(GenericRecompute):
    def __init__(self):
        super().__init__()

    def __str__(self):
        return "iterative a star"
    
    def recomputePath(self, pathRepr:ParseResult) -> int:
        print("Starting iterative A* recompute")
    
        newLen = 0
        for layerGenerator in pathRepr.layerGeneratorGenerator():
            # setup for this z layer

            layerList : List[gcodeSegment] = list(layerGenerator)
            layerList = list(filter(lambda x: x.isPrint(), layerList))

            print(f"{len(layerList)} segments in layer z={layerList[0].point[2]}")

            simpleIterativeAStar(layerList)
            break # TODO - refactor above and remove this

            # get the starting states for this input
            def pointsGenerator() -> Tuple[int, int, int]:
                '''Generate all the segment endpoints in this layer'''
                for seg in layerList:
                    yield seg.point
                    yield seg.pointTwo

            # set of all unique points in the segment
            startPoints : Set[Tuple[int, int, int]] = set()
            for point in pointsGenerator():
                startPoints.add(point)

            # generate all the start point pairs

            # TODO - need to somehow cache this one
            startMap = BitMap(len(layerList))

            stateCache = {} # cache of the bitmap state
            stateCache[hash(startMap)] = [startMap]

            def startStateGenerator():
                t = list(startPoints)
                for i in range(len(t)):
                    for j in range(i+1, len(t)):
                        if pointDistance(t[i], t[j]) >= MIN_SEPERATION_MM:
                            # create and yield the state
                            yield SearchState(layerList, stateCache)
                            raise NotImplementedError()

            # pack into indexible generator
            ssg = IndexableStateGenerator(startStateGenerator())


            pq = PriorityQueue()
            
            pq.push(ssg)

            # time for a*
            while len(pq) > 0:
                s : SearchState = pq.peek().next()
                if s is None:
                    pq.pop()
                    continue
                pq.updateFront()
                    
                if s.isGoal():
                    break

                # check/update state presence in visited list

                # push the successor state into the list
                pq.push(s.getSuccessorGenerator())


            # TODO - for debugging , remove later
            return newLen

        return newLen

        return super().recomputePath(pathRepr)



# TODO - this is debug, should be removed
#   but it makes it much easier to test with

if __name__ == "__main__":
    from gcodeParser import gcodePathParser

    parser = gcodePathParser()

    filepath = "gcodeSampleSet/873067.gcode"

    parseResult = parser.parseFile(filepath)
    ias = IterativeAStar()

    print(ias.recomputePath(parseResult))