# libraries for working with the file system

import os

# get all files of the type fileType in the searchDir
def allFilesInDirByType(searchDir, fileType) -> list:
    if(fileType[0] == '.'):
        fileType = fileType[1]

    if(len(fileType) <= 0):
        #actually, it could be: lets see if this is a long term problem
        raise IndexError(f"The file type cannot be empty")

    return [f for f in [searchDir+'/'+f for f in os.listdir(searchDir)] if
                os.path.isfile(f) and f[-len(fileType):] == fileType]


# get all files of the type recursively
# in alphabetical preorder ordering
def allFilesInRecurseByType(searchDir, fileType) -> list:
    files = allFilesInDirByType(searchDir, fileType)

    for f in files:
        yield f

    for d in [d for d in [searchDir+'/'+f for f in os.listdir(searchDir)]
                if os.path.isdir(d)]:

        gen = allFilesInRecurseByType(d, fileType)
        for f in gen:
            yield f
    
    return

# gcodeSampleSet/800473.gcode --> "800473"
def getNameFromPath(filePath) -> str:
    if filePath.find("/") != -1:
        return getNameFromPath(filePath[filePath.find("/")+1:])
    return filePath[:filePath.rfind('.')]
    

# TODO - unit testing
if __name__ == "__main__":
    assert(getNameFromPath("gcodeSampleSet/800473.gcode") == "800473")
    print("All test passed")