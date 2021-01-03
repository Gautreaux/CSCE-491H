#takes the gcode files and converts them into a 
#file where each line is a print segment encoded
#   x0 y0 x1 y1


from fileIterables import allFilesInRecurseByType, getNameFromPath
from gcodeParser import gcodePathParser, ParseResult
from gcodeSegment import gcodeSegment

SAMPLE_DIR = "gcodeSampleSet"
FILE_TYPE = "gcode"

Layer_DIR = "layerSampleSet"
LFILE_TYPE = "lyr"


def getLayerName(fid, layer):
    return f"{Layer_DIR}/{fid}_{layer}.{LFILE_TYPE}"

def segRepr(segment):
    def pointRepr(point):
        return f"{point[0]} {point[1]}"

    return f"{pointRepr(segment.point)} {pointRepr(segment.pointTwo)}"

#dump all the layers to a file
def dumpLayers(pathRepr, fid):
    for layerGenerator in pathRepr.layerGeneratorGenerator():
        # get the generator as a list
        layerList = list(layerGenerator)
        #filter to only the print segments
        layerList = list(filter(lambda x: x.isPrint(), layerList))

        layerName = getLayerName(fid, layerList[0].point[2])
        with open(layerName, 'w') as outfile:
            for segment in layerList:
                print(segRepr(segment), file=outfile)

parser = gcodePathParser()
for filePath in allFilesInRecurseByType(SAMPLE_DIR, FILE_TYPE):
    fileID = getNameFromPath(filePath)

    try:
        parseResult = parser.parseFile(filePath)
        
        dumpLayers(parseResult, fileID)
    except FileNotFoundError:
        print(f"\tFile {filePath} could not be found")
        continue