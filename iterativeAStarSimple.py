# TODO - refactor into iterativeAStar

from abc import ABC, abstractmethod
from typing import Dict, List, Optional, Set, Tuple

from bitmapLib import BitMap
from gcodeSegment import gcodeSegment
from vectorLib import pointDistance


LPI_MAP_TYPE = Optional[Dict[Tuple[int, int], List[int]]]
# stores a mapping of positions to coressponding segment index
LAYER_POS_INDEX_MAPPING : LPI_MAP_TYPE = None

#list of all positions that are in the system
LAYER_POS_LIST : List[Tuple[int, int]] = None

# list of all the segments in the current layer
LAYER_SEGMENTS_LIST : List[gcodeSegment] = None

# since bitmaps are generally ~ 200 bytes,
#   sotring pointers (8 bytes) is much better than storing copies of same map
# TODO - this is actually really complicated. May need a custom class?
BITMAP_CACHE : Set[BitMap]

MIN_SEPERATION_MM : int = 25

def buildLayerPosIndexMapping() -> None:
    '''Build the mapping of position to indexes in the layer list'''
    global LAYER_POS_INDEX_MAPPING
    global LAYER_SEGMENTS_LIST
    global LAYER_POS_LIST

    for i in range(len(LAYER_SEGMENTS_LIST)):
        segment : gcodeSegment = LAYER_SEGMENTS_LIST[i]
        for p in [segment.point, segment.pointTwo]:
            if p not in LAYER_POS_INDEX_MAPPING:
                LAYER_POS_INDEX_MAPPING[p] = []
                LAYER_POS_LIST.append(p)
            LAYER_POS_INDEX_MAPPING[p].append(i)


class SearchState():
    def __init__(self, a1Index : int, a2Index : int, bitmap : BitMap) -> None:
        self.a1Index = a1Index
        self.a2Index = a2Index
        self.bitmap = bitmap

    def priority(self) -> int:
        return 2*self.bitmap.getCount()

    def isValid(self) -> bool:
        '''Return True iff this state is valid'''
        return pointDistance(LAYER_POS_LIST[self.a1Index], LAYER_POS_LIST[self.a2Index]) >= MIN_SEPERATION_MM

    def __str__(self) -> str:
        global LAYER_POS_LIST
        return (f"state: {LAYER_POS_LIST[self.a1Index]} {LAYER_POS_LIST[self.a2Index]}"
                f" {self.bitmap.getCount()}/{len(LAYER_POS_LIST)}")

class OrderedGenerator(ABC):

    @abstractmethod
    def __init__(self) -> None:
        raise NotImplementedError()

    @abstractmethod
    def order(self) -> int:
        raise NotImplementedError()

    @abstractmethod
    def peek(self) -> SearchState:
        raise NotImplementedError()

    @abstractmethod
    def next(self) -> SearchState:
        raise NotImplementedError()

# generate and return all the start states
class StartStateOrderedGenerator(OrderedGenerator):
    '''State Generator that generates all start states'''
    def __init__(self) -> None:
        # can cache priority since all are the same
        
        # TODO - store/pull this from the bitmap cache
        global LAYER_SEGMENTS_LIST
        self.bitmap : BitMap = BitMap(len(LAYER_SEGMENTS_LIST))

        # all states, being starting states, have the same priority
        self.i = -1
        self.j = len(LAYER_SEGMENTS_LIST)

        self.allPriority = self.peek().priority

    def order(self) -> int:
        return self.allPriority

    def _getNextIJ(self, i ,j):
        '''Given an i and j, return the i,j pair that follows it, 
            raising stopiteration if there is none'''
        j += 1
        if j >= len(LAYER_SEGMENTS_LIST):
            i += 1
            if i > len(LAYER_SEGMENTS_LIST) - 1:
                raise StopIteration()
            j = i + 1
        return (i,j)

    def _next(self) -> SearchState:
        """Return the next state without checking its validity"""
        self.i, self.j = self._getNextIJ(self.i, self.j)
        return SearchState(self.i, self.j, self.bitmap)

    def next(self) -> SearchState:
        """Return the next valid state"""
        while True:
            s = self._next()
            if s.isValid():
                return s

    def peek(self) -> SearchState:
        """Peek the next valid state"""
        i = self.i
        j = self.j
        while True:
            i,j = self._getNextIJ(i, j)
            s = SearchState(i, j, self.bitmap)
            if s.isValid():
                return s
            # since we know that i,j is not valid, we can save some time on the next next() by
            # updating the generator's behavior now
            # self.i = i
            # self.j = j
    
# class SuccessorStateOrderedGenerator(OrderedGenerator):
#     '''Sate generator that generates valid successors of the input state'''
#     def __init__(self, parentState) -> None:
#         raise NotImplementedError()
    
#     def order(self) -> int:
#         return self.peek().priority()


def isGoalSate(state : SearchState) -> bool:
    '''Return True if this is a goal state'''
    return state.bitmap.isFull()

def simpleIterativeAStar(segmentList : List[gcodeSegment]) -> int:
    """do the iterative AStar and return the time for this one layer"""

    global LAYER_POS_INDEX_MAPPING
    global LAYER_POS_LIST
    global LAYER_SEGMENTS_LIST
    global BITMAP_CACHE

    # reset the global variables
    BITMAP_CACHE = set()
    LAYER_SEGMENTS_LIST = []
    LAYER_POS_LIST = []
    LAYER_POS_INDEX_MAPPING = {}
    
    # put this into the global scope to make things easier
    LAYER_SEGMENTS_LIST = segmentList

    # build a mapping of the starting index to the next index
    buildLayerPosIndexMapping()

    # DEBUG
    # print(LAYER_POS_INDEX_MAPPING)
    # print(LAYER_POS_LIST)

    # starting state generator
    ss_gen = StartStateOrderedGenerator()
    # TESTED: OK

    print("Layer setup complete")

    while True:
        print(ss_gen.next())
