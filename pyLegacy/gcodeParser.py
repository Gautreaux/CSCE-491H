# code for parsing a gcode file
from abc import ABC, abstractmethod

from gcodeSegment import gcodeSegment

# any derived class must be multi-threaded/multi-processed safe
class gcodeParserGeneric(ABC):
    def __init__(self):
        self.registerOnEmptyLine(None)
        self.registerOnCommentLine(None)
        self.registerOnInvalidLine(None)
        self.registerOnAllLine(None)
        self.gcodeCallbacks = {}
        self.gcodeNOOP = set()

    # register a new function for processing empty lines
    # function should be self->void
    # pass None to unregister
    def registerOnEmptyLine(self, onEmptyLine):
        self.onEmptyLine = onEmptyLine

    # register a new function for processing whole-line comment
    # function should be self,line->void
    # pass None to unregister
    def registerOnCommentLine(self, onCommentLine):
        self.onCommentLine = onCommentLine

    # register a new function for processing an invalid line
    # function should be self,line->void
    # pass None to unregister
    def registerOnInvalidLine(self, onInvalidLine):
        self.onInvalidLine = onInvalidLine
    
    # register a new function for processing every line
    # function should be self,line->void
    # pass None to unregister
    def registerOnAllLine(self, onAllLine):
        self.onAllLine = onAllLine

    # register a new function to process a line based on a code
    # function should be self,line->void
    # pass None to unregister
    def registerOnCode(self, code, callback):
        # should be uppercase, but good to enforce
        self.gcodeCallbacks[code.capitalize()] = callback

    # register a no-op (pass) for a specific gcode
    # can pass a iterable of str or individual
    def registerNOOP(self, code):
        if type(code) == str:
            self.gcodeNOOP.add(code.capitalize())
        else:
            for c in code:
                self.gcodeNOOP.add(c.capitalize())

    # get the operation of a line
    def getOP(self, line):
        return line.split(' ', 1)[0].capitalize()
    
    def processLine(self, line):
        if self.onAllLine is not None:
            self.onAllLine(line)

        if len(line) == 0:
            if self.onEmptyLine is not None:
                self.onEmptyLine()
            return

        op = self.getOP(line)
        if (op in self.gcodeCallbacks and
                self.gcodeCallbacks[op] is not None):
            self.gcodeCallbacks[op](line)
        elif op == ';':
            # TODO - this should process on the tail of all lines just in case
            if self.onCommentLine is not None:
                self.onCommentLine(line)
        elif op in self.gcodeNOOP:
            pass
        else:
            if self.onInvalidLine is not None:
                self.onInvalidLine(line)
        
    def parseFile(self, filepath):
        with open(filepath) as gcodeFile:
            self.startFile(filepath)

            for line in gcodeFile:
                self.processLine(line.strip())

            return self.endFile(filepath)
    
    # reset the output data structure
    @abstractmethod
    def startFile(self):
        return

    # get the output data structure
    @abstractmethod
    def endFile(self):
        return

# a parser which prints file stats
# and returns a list of file segments
class gcodePathParser(gcodeParserGeneric):
    def __init__(self):
        super().__init__()
        self.registerOnInvalidLine(self.invalidCallback)
        self.registerOnAllLine(self.allCallback)
        self.registerOnCode("G1", self.g1callback)
        self.registerOnCode("G20", self.g20callback)
        self.registerOnCode("G21", self.g21callback)
        self.registerOnCode("G28", self.g28callback)
        self.registerOnCode("G90", self.g90callback)
        self.registerOnCode("G91", self.g91callback)
        self.registerNOOP([
                "M140", "M84", "M101", "M103", "M73", "M104", "M126", "M127"
        ])

    def startFile(self, _):
        self.output = ParseResult()
        self.pos = (0,0,0)
        self.homedState = [0,0,0]
        self.movementsBeforeHomed = 0
        self.absPos = True
        self.metricUnits = True
        self.lastExtrude = 0
    
    def endFile(self, _):
        self.output.homedState = self.homedState
        self.output.movementsBeforeHomed = self.movementsBeforeHomed

        #cleanup the segment list to remove the lead in/out movements
        i = 0
        while i < len(self.output.segList):
            if self.output.segList[i].isPrint():
                break
            i += 1
        self.output.segList = self.output.segList[i:]

        i = len(self.output.segList) - 1
        while i >= 0:
            if self.output.segList[i].isPrint():
                break
            i -= 1
        self.output.segList = self.output.segList[:i+1]

        return self.output
    
    def allCallback(self, _):
        self.output.totalLines += 1

    def invalidCallback(self, line):
        self.output.uncaughtCodes.add(self.getOP(line))
        self.output.invalidLines += 1

    #standard linear interpolation move
    def g1callback(self, line):

        if 0 in self.homedState:
            self.output.movementsBeforeHomed += 1
            return

        values = line.split(" ")
        s = {}

        for v in values:
            if v == ';':
                break
            s[v[0].capitalize()] = float(v[1:])
        
        if self.metricUnits is False:
            raise NotImplementedError("Imperial Units are not supported")
        
        if self.absPos is True:
            newPos = (
                    s['X'] if 'X' in s else self.pos[0],
                    s['Y'] if 'Y' in s else self.pos[1],
                    s['Z'] if 'Z' in s else self.pos[2]
            )
        else:
            #relative mode
            newPos = (
                self.pos[0] + s['X'] if 'X' in s else 0,
                self.pos[1] + s['Y'] if 'Y' in s else 0,
                self.pos[2] + s['Z'] if 'Z' in s else 0,
            )

        if newPos == self.pos:
            # this segment is doing nothing with X,Y,Z
            # i.e. its doing some extruder only operation
            # so we will skip it
            return

        newExtrude = float(s['E']) if 'E' in s else None
        if newExtrude is not None:
            printAmt = newExtrude - self.lastExtrude
            self.lastExtrude = newExtrude
        else:
            printAmt = None

        #create
        thisSegment =  gcodeSegment(self.pos, newPos, printAmt)

        self.output.segList.append(thisSegment)

        self.pos = newPos

    # set to imperial  
    def g20callback(self, _):
        self.metricUnits = False
    
    # set to metric
    def g21callback(self, _):
        self.metricUnits = True
    
    # home axes
    def g28callback(self, line):
        newLine = line[:line.find(';')].strip()
        if len(newLine.split(" ")) == 1:
            self.homedState = [1,1,1]
            self.pos = (0,0,0)
        else:
            #TODO - is this being done correctly?
            for v in newLine.split(" "):
                if v.capitalize() == "G28":
                    continue
                try:
                    self.homedState[{'X':0, 'Y':1, 'Z':2}[v[0].capitalize()]] = 1
                except KeyError:
                    pass                

    # switch to ABS Position Mode
    def g90callback(self, _):
        self.absPos = True

    # switch to Relative Position Mode
    def g91callback(self, _):
        self.absPos = False


class ParseResult():
    def __init__(self):
        self.totalLines = 0  # total lines in file
        self.invalidLines = 0  # count lines with a parse error
        self.segList = []
        self.uncaughtCodes = set()
        self.homedState = [0,0,0]
        self.movementsBeforeHomed = -1
        self.layerSkipList = None

    def getReport(self):
        print(f"Total Lines: {self.totalLines}")
        print(f"Invalid Lines: {self.invalidLines}")
        
        print(f"Resolved Segments (total): {len(self.segList)}")
        print("Resolved Segments - print: "
                f"{sum(map(lambda x: 1 if x.isPrint() else 0, self.segList))}")
        print("Resolved Segments - travel: "
                f"{sum(map(lambda x: 0 if x.isPrint() else 1, self.segList))}")
        if len(self.uncaughtCodes) > 0:
            print(f"WARNING: {len(self.uncaughtCodes)} uncaught codes found.")
            print(self.uncaughtCodes)
        if self.movementsBeforeHomed != 0:
            print(f"WARNING: {len(self.movementsBeforeHomed)} movements before homing")
        if 0 in self.homedState:
            print(f"WARNING: the gcode file did not contain a home instruction")

    #iterate over all segments
    def __iter__(self):
        return self.segList.__iter__()
    
    def __len__(self):
        return len(self.segList)

    def __getitem__(self, key):
        return self.segList[key]

    def getZLayers(self) -> set:
        if self.layerSkipList == None:
            self.buildLayerSkipList()
        return set(self.layerSkipList)

    def buildLayerSkipList(self):
        self.layerSkipList = {}

        def getZ(seg):
            return seg.point[2]

        ctr = 0
        segLen = len(self.segList)

        if(segLen == 0):
            raise ValueError("Total zero segments in parse result")

        # removing skirt
        # iterate to the first print segment
        while(ctr < segLen):
            
            if self.segList[ctr].isPrint():
                break
            ctr += 1

        # removing skirt        
        # iterate to the first non-print
        while (ctr < segLen):
            if not self.segList[ctr].isPrint():
                break
            ctr += 1

        if(ctr == segLen):
            raise ValueError("Something went wrong while parsing out the skirt")

        # parse out the layers
        while (ctr < segLen):
            #index to the next print element
            while (ctr < segLen):
                if self.segList[ctr].isPrint():
                    break
                ctr += 1
            if(ctr == segLen):
                raise ValueError("Something went wrong parsing layer")

            zLayer = getZ(self.segList[ctr])
            startInd = ctr
            lastInd = startInd 
            while(ctr < segLen):
                # assumes that no Z-Hop is performed
                if getZ(self.segList[ctr]) != zLayer:
                    break
                if self.segList[ctr].isPrint():
                    lastInd = ctr
                ctr += 1
            
            if zLayer in self.layerSkipList:
                raise ValueError(f"The z layer z={zLayer} is already in skip list")
            self.layerSkipList[zLayer] = (startInd, lastInd+1)


    # a generator for all the segments in a particular z layer
    #   zLayer should be in the getZLayers
    def layerGenerator(self, zLayer):
        if self.layerSkipList == None:
            self.buildLayerSkipList()

        if zLayer not in self.getZLayers():
            return ValueError("Invalid zLayer provided as input")
        
        (s,e) = self.layerSkipList[zLayer]
        for i in range(s,e):
            yield self.segList[i]
    

    # a generator that returns generators for all z layers
    def layerGeneratorGenerator(self):
        if self.layerSkipList == None:
            self.buildLayerSkipList()
        
        for layer in self.getZLayers():
            yield self.layerGenerator(layer)


# light unit testing, TODO - improve
if __name__ == "__main__":
    from testUtils import getFirstGCodePath
    
    firstPath = getFirstGCodePath()


    parser = gcodePathParser()
    parseResult = parser.parseFile(firstPath)

    parseResult.getReport()

    print(f"Total Segments: {len(parseResult)}")
    print(f"Total print Len: {sum(map(lambda x: 0 if x is None else x, map(lambda x: x.printAmt, parseResult)))}")
    layers = parseResult.getZLayers()
    print(f"Z layers: {layers}")

    layerList = list(layers)

    #TODO - layer generator testing