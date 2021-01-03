#various functions for the testing utility

def getFirstGCodePath():
    from fileIterables import allFilesInRecurseByType
    
    # get the first gcode
    DIR = "gcodeFullSet"
    gen = allFilesInRecurseByType("gcodeFullSet", "gcode")
    return next(gen)


def getRandPath():
    raise NotImplementedError()