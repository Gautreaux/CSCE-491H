#copy a subset of the files from the all gcode to the sample set
from fileIterables import *
from random import sample
from shutil import copy2

FULL_GCODE_DIR = "gcodeFullSet"
SAMPLE_TARGET_DIR = "gcodeSampleSet"
GCODE_EXTENSION = "gcode"


def getGCodeCount(searchDir=FULL_GCODE_DIR) -> int:
    return sum(1 for x in allFilesInRecurseByType(searchDir, GCODE_EXTENSION))

def getGCodePaths(searchDir=FULL_GCODE_DIR) -> list:
    return list(allFilesInRecurseByType(searchDir, GCODE_EXTENSION))


if __name__ == "__main__":
    print(f"Total files in full set: {getGCodeCount()}")

    samples = sample(getGCodePaths(), 15)

    for s in samples:
        copy2(s, SAMPLE_TARGET_DIR)
