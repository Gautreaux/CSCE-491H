# TODO - refactor into iterativeAStar

from abc import ABC, abstractmethod
from copy import deepcopy
import itertools

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

# several extremely helpful functions

def getIncidentSegmentIndiciesFromPosition(pos : Tuple[int, int]) -> List[int]:
    """get all indexes of segments incident on the position"""
    global LAYER_POS_INDEX_MAPPING
    return LAYER_POS_INDEX_MAPPING[pos]

def getPositionFromPostionIndex(posIndex : int) -> Tuple[int, int]:
    """get the position associated with a position Index"""
    global LAYER_POS_LIST
    return LAYER_POS_LIST[posIndex]

def getInicidentSegmentIndiciesFromPositionIndex(posIndex : int) -> List[int]:
    """"get all indexes of segments incident on a position index """
    global LAYER_POS_LIST
    global LAYER_POS_INDEX_MAPPING
    return LAYER_POS_INDEX_MAPPING[LAYER_POS_LIST[posIndex]]


class SearchState():
    def __init__(self, a1Index : int, a2Index : int, bitmap : BitMap) -> None:
        self.a1Index = a1Index
        self.a2Index = a2Index
        self.bitmap = bitmap

    def priority(self) -> int:
        return 2*self.bitmap.getCount()

    def isValid(self) -> bool:
        global LAYER_POS_LIST
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

    # derived generators may be able to provide a better priority
    def order(self) -> int:
        return self.peek().priority()

    # derived generators may be able to provide a more efficient implementation
    def peek(self) -> SearchState:
        return deepcopy(self).next()

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
    
class SuccessorStateOrderedGenerator(OrderedGenerator):
    '''Sate generator that generates valid successors of the input state'''
    def __init__(self, parentState : SearchState) -> None:
        self.parentState = parentState
        self.mode = 1
        self.i = None


    def _getNext1(self) -> Tuple[int, int]:
        """return segment indexes of the next type 1 successors"""
        if self.i is None:
            a1Incident = getInicidentSegmentIndiciesFromPositionIndex(self.parentState.a1Index)
            a2Incident = getInicidentSegmentIndiciesFromPositionIndex(self.parentState.a2Index)
            self.i = itertools.product(a1Incident, a2Incident)
        return next(self.i)

    def _getNext2(self) -> Tuple[Optional[int], Optional[int]]:
        global LAYER_SEGMENTS_LIST
        if self.i is None:
            a1Incident = getInicidentSegmentIndiciesFromPositionIndex(self.parentState.a1Index)
            a2Incident = getInicidentSegmentIndiciesFromPositionIndex(self.parentState.a2Index)
            i1 = itertools.product(a1Incident, [None])
            i2 = itertools.product([None], a2Incident)
            i3 = itertools.product(a1Incident, range(len(LAYER_SEGMENTS_LIST)))
            i4 = itertools.product(range(len(LAYER_SEGMENTS_LIST)), a2Incident)
            self.i = itertools.chain(i1,i2, i3, i4)
        return next(self.i)

    def next(self) -> SearchState:
        # so what we want to do is generate states in the following order:
        #   1. generate all states where both agents can print something
        #   2. generate all states where one agent can print something
        #           and the other agent is either NOP or movement
        #   3. generate all states where both agents are movement
        #           or one agent moves and the other is NOP
        #   There may be some implied priority in 2/3 b/t NOP and movement
        #       but thats a future problem
        global LAYER_SEGMENTS_LIST
        global LAYER_POS_LIST

        if self.mode == 1:
            try:
                while True:
                    segA1Index, segA2Index = self._getNext1()

                    # check if the state was already printed
                    if self.parentState.bitmap[segA1Index] == 1:
                        # it was already printed, so this transition is not valid
                        continue
                    if self.parentState.bitmap[segA2Index] == 1:
                        continue

                    # get the segment objects
                    segA1 = LAYER_SEGMENTS_LIST[segA1Index]
                    segA2 = LAYER_SEGMENTS_LIST[segA2Index]

                    # check if the segments are separated by the proper amount
                    if segA1.minSeperation(segA2) < MIN_SEPERATION_MM:
                        # there is not enough seperation
                        continue

                    # segment are separated by enough
                    #   which implies the endpoints are separated by enough
                    # so lets create a new state
                    newBitmap = deepcopy(self.parentState.bitmap)
                    newBitmap[segA1Index] = 1
                    newBitmap[segA2Index] = 1

                    oldA1Pos = LAYER_POS_LIST[self.parentState.a1Index]
                    oldA2Pos = LAYER_POS_LIST[self.parentState.a2Index]

                    # get the point opposite where we start
                    newA1Pos = (segA1.point if segA1.pointTwo == oldA1Pos else segA1.pointTwo)
                    newA2Pos = (segA2.point if segA2.pointTwo == oldA2Pos else segA2.pointTwo)

                    #TODO - this could be pre-computed 
                    newA1Index = LAYER_POS_LIST.index(newA1Pos)
                    newA2Index = LAYER_POS_LIST.index(newA2Pos)

                    return SearchState(newA1Index, newA2Index, newBitmap)
            except StopIteration:
                # switch to mode 2 and recurse
                self.mode = 2
                self.i = None
                return self.next()
        elif self.mode == 2:
            # todo - implement mode 2
            try:
                segA1Index, segA2Index = self._getNext2()
                # one of the two is none,
                #   the other is an segment index incident on one of the positions

                raise NotImplementedError()
            except StopIteration:
                # switch to mode 3 and recurse
                self.mode = 3
                self.i = None
                return self.next()
        else: # mode == 3
            #TODO - implement mode 3    
            raise NotImplementedError()


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

    # while True:
    #     print(ss_gen.next())

    ss = ss_gen.next()


    sss = SuccessorStateOrderedGenerator(ss)
    while True:
        print(sss.next())