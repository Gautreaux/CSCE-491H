# class for a path of gcode segments

from gcodeSegment import gcodeSegment

# raised when the path displays disjoint behavior
class DisjointPathException(Exception):
    pass

class GCodeSegPath():
    # should this extend a LineSegmentPath class?
    # TODO - the parser should be rewritten as well as the recompute things
    #       that operate on paths or path segment

    def __init__(self, fromIterable=None, args=None):
        self.segList = []

        if fromIterable is not None:
            self.__buildFromIterable(fromIterable, args)            
        elif(False):
            # placeholder for future extensibility
            pass

        raise Exception("Could not find a valid constructor framework")
        
    # build from a stateful/stateless iterable
    #   args - a single float for max len, else None
    def __buildFromIterable(self, iterable, args):
        isFirst = False

        for segment in iterable:
            assert(type(segment) == gcodeSegment)

            # just kind of assume the segments are in order
            #   we check later to be sure
            if args is not None:
                #do the split behavior
                s2 = None # TODO
                raise NotImplementedError()
            else:
                s2 = segment
            
            self.segList.append(s2)
        
        #finally, check the constructed path for consistency
        try:
            self.checkPathConsistency()
        except DisjointPathException:
            print("Warning, after constructing path, the resulting path"
                    "was disjoint")
        
    # check certain conditions on the path:
    #   1. the start of all paths is the end of the previous one
    # returns True on success
    # raises DisjointPathException on failure
    def checkPathConsistency(self):
        raise NotImplementedError()

    # i think this is helpful for iteratively building the list
    def appendSegment(self, seg):
        if type(seg) is not gcodeSegment:
            raise TypeError(f"Illegal type {type(seg)}. Expected {gcodeSegment}")

        # TODO - some consistency check to endure we are maintaining the list
        raise NotImplementedError()

        self.segList.append(seg)

    def __iter__(self):
        return self.segList.__iter__()
    
    def __len__(self):
        return len(self.segList)

    def __getitem__(self, key):
        return self.segList[key]

# TODO - unit testing