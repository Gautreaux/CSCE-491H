
from fileIterables import allFilesInRecurseByType, getNameFromPath
from gcodeParser import gcodePathParser
from recomputeFramework import BatchRecompute

# from recomputeTrivial import TrivialRecompute, InvalidRecompute
from recomputeFundamentals import PrintTimeRecompute, PrintTravelTimeRecompute
from recomputeODonnell import InwardODonnell, InwardOdonnellSplit

SAMPLE_DIR = "gcodeSampleSet"
FILE_TYPE = "gcode"

HEADER_STR = "Header:[printDist,NormalDist,Odonnel,SplitOdonnel]"

if __name__ == "__main__":
    print("Lets go!")

    parser = gcodePathParser()
    worker = BatchRecompute()
    worker.addRecomputeModel(PrintTimeRecompute)
    worker.addRecomputeModel(PrintTravelTimeRecompute)
    worker.addRecomputeModel(InwardODonnell)
    worker.addRecomputeModel(InwardOdonnellSplit)

    parseValid = {}
    recomputeResults = {}

    i = 0
    for filePath in allFilesInRecurseByType(SAMPLE_DIR, FILE_TYPE):
        i += 1

        if i not in [2]:
            continue
        print("For debug testing, stopping after 2 file")
        
        print(f"Starting file {i}:{filePath}")
        fileID = getNameFromPath(filePath)

        # default values
        parseValid[fileID] = False
        recomputeResults[fileID] = None

        def printRes(header=None):
            if header != None:
                print(header)

            for key in parseValid:
                print(f"{key}:[", end="")
                baseTime = recomputeResults[key][1]
                if parseValid[key] is False:
                    print("parse failure")
                else:   
                    for k in recomputeResults[key]:
                        try:
                            float(baseTime)
                            print(f"{round(k)} ({round(k/baseTime*100,2)}%),", end="")
                        except TypeError:
                            print(f"{round(k)}", end="")
                print("]")

        try:
            parseResult = parser.parseFile(filePath)
            
            #TODO - should do some better check here?

            parseValid[fileID] = True
        except FileNotFoundError:
            print(f"\tFile could not be found")
            continue

        print(f"\tParsing completed: {len(parseResult)} segments; starting recompute")

        try:
            recomputeResults[fileID] = worker.doBatchProcess(parseResult, suppressException=False)
        except Exception as e:
            print(f"An exception occurred, dumping partial compute response then throwing.")
            printRes(HEADER_STR)
            raise e

    printRes(HEADER_STR)