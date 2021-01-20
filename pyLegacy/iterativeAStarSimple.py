# TODO - refactor into iterativeAStar

from abc import ABC, abstractmethod
from copy import deepcopy
import itertools
from os import stat

from typing import Dict, List, Optional, Set, Tuple

from bitmapLib import BitMap
from gcodeSegment import gcodeSegment
from priorityQueueLib import PriorityQueue
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
    def __init__(self, a1Index : int, a2Index : int, bitmap : BitMap, depth) -> None:
        self.a1Index = a1Index
        self.a2Index = a2Index
        self.bitmap = bitmap
        self.depth = depth

    def priority(self) -> int:
        # DFS search
        # return len(LAYER_POS_LIST) - self.bitmap.getCount()

        # BFS?
        # return 2*self.bitmap.getCount()

        # A*
        return self.depth + 2*self.bitmap.getCount()

    def isValid(self) -> bool:
        global LAYER_POS_LIST
        '''Return True iff this state is valid'''
        return pointDistance(LAYER_POS_LIST[self.a1Index], LAYER_POS_LIST[self.a2Index]) >= MIN_SEPERATION_MM

    def __str__(self) -> str:
        global LAYER_POS_LIST
        return (f"state: {LAYER_POS_LIST[self.a1Index]} {LAYER_POS_LIST[self.a2Index]}"
                f" {self.bitmap.getCount()}/{len(LAYER_POS_LIST)} {self.depth}")

    def __hash__(self) -> int:
        return hash(self.bitmap)

    def get_pos_tuples(self) -> Tuple[int, int]:
        return (self.a1Index, self.a2Index)

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

    
    def __lt__(self, o: object) -> bool:
        return self.order() < o.order()


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

        self.allPriority = self.peek().priority()

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
        return SearchState(self.i, self.j, self.bitmap, 0)

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
            s = SearchState(i, j, self.bitmap, 0)
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
        """return segment indexes of the successor indexes to print"""
        if self.i is None:
            a1Incident = getInicidentSegmentIndiciesFromPositionIndex(self.parentState.a1Index)
            a2Incident = getInicidentSegmentIndiciesFromPositionIndex(self.parentState.a2Index)
            self.i = itertools.product(a1Incident, a2Incident)
        return next(self.i)

    def _getNext2(self) -> Tuple[Optional[int], Optional[int]]:
        """return a pair None and Int where
            None is a NOP
            int is the segment index to print"""
        if self.i is None:
            a1Incident = getInicidentSegmentIndiciesFromPositionIndex(self.parentState.a1Index)
            a2Incident = getInicidentSegmentIndiciesFromPositionIndex(self.parentState.a2Index)
            i1 = itertools.product(a1Incident, [None])
            i2 = itertools.product([None], a2Incident)
            self.i = itertools.chain(i1, i2)
        return next(self.i)

    def _getNext3(self) -> Tuple[int, int]:
        """returns a pair of positive and negative int where
            positive int is the segment index to print
            negative int is the position index to move to"""
        global LAYER_POS_LIST
        if self.i is None:
            a1Incident = getInicidentSegmentIndiciesFromPositionIndex(self.parentState.a1Index)
            a2Incident = getInicidentSegmentIndiciesFromPositionIndex(self.parentState.a2Index)
            l = len(LAYER_POS_LIST)
            # TODO - this can return a negative 0 and then what?
            i1 = itertools.product(a1Incident, range(0, -l, -1))
            i2 = itertools.product(range(0, -l, -1), a2Incident)
            self.i = itertools.chain(i1, i2)
        return next(self.i)

    def _getNext4(self) -> Tuple[Optional[int], Optional[int]]:
        """returns a pair int and None where
            None is a NOP
            int is the position index to move to"""
        global LAYER_POS_LIST
        if self.i is None:
            i1 = itertools.product(range(len(LAYER_POS_LIST)), [None])
            i2 = itertools.product([None], range(len(LAYER_POS_LIST)))
            self.i = itertools.chain(i1, i2)
        return next(self.i)

    def _getNext5(self) -> Tuple[int, int]:
        """returns a pair of int where each int is the position index to move to"""
        global LAYER_POS_LIST
        if self.i is None:
            self.i = itertools.product(range(len(LAYER_POS_LIST)), range((len(LAYER_POS_LIST))))
        return next(self.i)

    def _doMode1Check(self) -> Optional[SearchState]:
        """Do the checking for mode 1 and return the state,
            returning None if invalid state"""
        segA1Index, segA2Index = self._getNext1()

        # check if the state was already printed
        if self.parentState.bitmap[segA1Index] == 1:
            # it was already printed, so this transition is not valid
            return None
        if self.parentState.bitmap[segA2Index] == 1:
            return None

        # get the segment objects
        segA1 = LAYER_SEGMENTS_LIST[segA1Index]
        segA2 = LAYER_SEGMENTS_LIST[segA2Index]

        # check if the segments are separated by the proper amount
        if segA1.minSeperation(segA2) < MIN_SEPERATION_MM:
            # there is not enough seperation
            return None

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

        #TODO - this should be pre-computed 
        newA1Index = LAYER_POS_LIST.index(newA1Pos)
        newA2Index = LAYER_POS_LIST.index(newA2Pos)

        return SearchState(newA1Index, newA2Index, newBitmap, self.parentState.depth + 1)

    def _doMode2Check(self) -> Optional[SearchState]:
        """Do the checking for mode 2 and return the state,
            returning None if invalid state"""
        global LAYER_SEGMENTS_LIST
        global LAYER_POS_LIST

        segA1Index, segA2Index = self._getNext2()

        pointACommIndex = (self.parentState.a1Index if segA1Index is None else self.parentState.a2Index)
        oldPosIndex = (self.parentState.a1Index if segA2Index is None else self.parentState.a2Index)
        segACommIndex = (segA1Index if segA1Index is not None else segA2Index)
        if self.parentState.bitmap[segACommIndex] == 1:
            return None

        segAComm = LAYER_SEGMENTS_LIST[segACommIndex]
        pointAComm = LAYER_POS_LIST[pointACommIndex]

        if segAComm.minSeperationPoint(pointAComm) < MIN_SEPERATION_MM:
            return None

        oldPosComm = LAYER_POS_LIST[oldPosIndex]

        newBitmap = deepcopy(self.parentState.bitmap)
        newBitmap[segACommIndex] = 1

        newPos = (segAComm.point if segAComm.pointTwo == oldPosComm else segAComm.pointTwo)

        # TODO - this should be precomputed
        newIndex = LAYER_POS_LIST.index(newPos)

        if segA1Index is None:
            newA1Index = pointACommIndex
            newA2Index = newIndex
        else:
            newA2Index = pointACommIndex
            newA1Index = newIndex

        return SearchState(newA1Index, newA2Index, newBitmap, self.parentState.depth + 1)

        raise NotImplementedError()

    def _doMode3Check(self) -> Optional[SearchState]:
        """Do the checking for mode 3 and return the state,
            returning None if invalid state"""
        segA1Index, segA2Index = self._getNext1()

        printSegIndex = segA1Index if segA1Index >= 0 else segA2Index
        positionEndIndex = -segA1Index if segA1Index < 0 else -segA2Index
        postionStartIndex = self.parentState.a1Index if segA1Index < 0 else self.parentState.a2Index

        if postionStartIndex == positionEndIndex:
            return None

        if self.parentState.bitmap[printSegIndex] == 1:
            return None

        printSeg = LAYER_SEGMENTS_LIST[printSegIndex]
    
        # construct the sudo segment for the movement
        posStart = LAYER_POS_LIST[postionStartIndex]
        posEnd = LAYER_POS_LIST[positionEndIndex]
        posSeg = gcodeSegment(posStart, posEnd)

        if printSeg.minSeperation(posSeg) < MIN_SEPERATION_MM:
            return None

        #create new state
        newBitmap = deepcopy(self.parentState.bitmap)
        newBitmap[printSegIndex] = 1

        oldPosIndex = self.parentState.a1Index if segA1Index >= 0 else self.parentState.a2Index
        oldPos = LAYER_POS_LIST[oldPosIndex]
        newPos = printSeg.point if oldPos == printSeg.pointTwo else printSeg.pointTwo

        newIndex = LAYER_POS_LIST.index(newPos)

        if segA1Index >= 0:
            newA1Index = newIndex
            newA2Index = positionEndIndex
        else:
            newA1Index = positionEndIndex
            newA2Index = newIndex

        return SearchState(newA1Index, newA2Index, newBitmap, self.parentState.depth + 1)

    def _doMode4Check(self) -> Optional[SearchState]:
        """Do the checking for mode 4 and return the state,
            returning None if invalid state"""
        
        segA1Index, segA2Index = self._getNext4()

        nopPosIndex = self.parentState.a1Index if segA1Index is None else self.parentState.a2Index
        movePosTargetIndex = segA1Index if segA1Index is not None else segA2Index
        movePosStartIndex = self.parentState.a1Index if segA1Index is not None else self.parentState.a2Index

        if movePosStartIndex == movePosTargetIndex:
            return None

        movePosTarget = LAYER_POS_LIST[movePosTargetIndex]
        movePosStart = LAYER_POS_LIST[movePosStartIndex]
        nopPos = LAYER_POS_LIST[nopPosIndex]

        dummySeg = gcodeSegment(movePosStart, movePosTarget)

        if dummySeg.minSeperationPoint(nopPos) < MIN_SEPERATION_MM:
            return None

        if segA1Index is None:
            newA1Index = nopPosIndex
            newA2Index = movePosTargetIndex
        else:
            newA1Index = movePosTargetIndex
            newA2Index = nopPosIndex
        
        return SearchState(newA1Index, newA2Index, self.parentState.bitmap, self.parentState.depth + 1)

    def _doMode5Check(self) -> Optional[SearchState]:
        """Do the checking for mode 5 and return the state,
            returning None if invalid state"""

        posA1TargetIndex, posA2TargetIndex = self._getNext5()

        posA1StartIndex = self.parentState.a1Index
        posA2StartIndex = self.parentState.a2Index

        if posA1TargetIndex == posA1StartIndex:
            return None
        if posA2TargetIndex == posA2StartIndex:
            return None

        posA1TargetPos = LAYER_POS_LIST[posA1TargetIndex]
        posA1StartPos = LAYER_POS_LIST[posA1StartIndex]
        posA2TargetPos = LAYER_POS_LIST[posA2TargetIndex]
        posA2StartPos = LAYER_POS_LIST[posA2StartIndex]

        dummmySegA1 = gcodeSegment(posA1StartPos, posA1TargetPos)
        dummmySegA2 = gcodeSegment(posA2StartPos, posA2TargetPos)

        if dummmySegA1.minSeperation(dummmySegA2) < MIN_SEPERATION_MM:
            return None
        
        return SearchState(posA1TargetIndex, posA2TargetIndex, self.parentState.bitmap, self.parentState.depth + 1)



        raise NotImplementedError()

    def next(self) -> SearchState:
        # so what we want to do is generate states in the following order:
        #   1. generate all states where both agents can print something
        #   2. generate all states where one agent can print something
        #       and the other is NO-OP
        #   3. generate all states where one agent can print something
        #       and the other is moving to a new location
        #   4. generate all states where one agent are movement
        #       and the other is NOP
        #   5. generate all states where both agents are moving to new locations
        #   There may be some implied priority in 2/3 b/t NOP and movement
        #       but thats a future problem
        global LAYER_SEGMENTS_LIST
        global LAYER_POS_LIST

        try:
            functionPointers = [0, self._doMode1Check, self._doMode2Check, self._doMode3Check,
                                self._doMode4Check, self._doMode5Check]
            while True:
                t = functionPointers[self.mode]()
                if t != None:
                    return t
        except StopIteration:
            # switch to mode 2 and recurse
            self.mode += 1
            if(self.mode > 5):
                raise StopIteration()
            self.i = None
            return self.next()


class VisitedStates():
    def __init__(self) -> None:
        self.d = {}

    def stateIn(self, state: SearchState) -> bool:
        h = hash(state)
        if h not in self.d:
            return False
        else:
            return state.get_pos_tuples() in self.d[h]

    def addState(self, state: SearchState) -> None:
        if self.stateIn(state):
            return
        
        h = hash(state)
        if h not in self.d:
            self.d[h] = set()
        self.d[h].add(state.get_pos_tuples())

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


    pq = PriorityQueue()
    vs = VisitedStates()
    pq.push(ss_gen)
    foundGoal = False
    maxDepth = -1

    while len(pq) > 0:
        thisGen : OrderedGenerator = pq.pop()
        try:
            thisState = thisGen.next()
        except StopIteration:
            # there is no next state
            continue

        try:
            pq.push(thisGen)
        except StopIteration:
            # there will be no next state
            pass

        if isGoalSate(thisState):
            print(f"Found a goal state at depth {thisState.depth}")
            foundGoal = True
            break
            
        if vs.stateIn(thisState):
            continue
        
        vs.addState(thisState)

        if thisState.depth > maxDepth:
            maxDepth = thisState.depth
            print(f"Found new max depth {maxDepth}, q size {len(pq)}, {thisState}")

        ss = SuccessorStateOrderedGenerator(thisState)

        pq.push(ss)

    if foundGoal is False:
        print("The algorithm returned false before a goal state could be found")