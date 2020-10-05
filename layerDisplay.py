# tools for displaying a layer

import matplotlib.pyplot as plt
from matplotlib import cm

# draw/display a layer
#   iterable - an interable of GcodeSegment
#   toolRadius - the radius of the tool to draw on the image
def displayLayer(iterable, toolRadius=None):
    plt.figure()

    #need to become stateless iterator
    if type(iterable) != list:
        segList = list(iterable)
    else:
        segList = iterable

    segCount = len(segList)

    if segCount <= 0:
        raise ValueError("SegCount must be positive number")

    i = 0
    for seg in segList:
        (x, y) = seg.asXYPlotPair()
        plt.plot(x,y, color=cm.jet(i/segCount))
        i += 1
    
    ax = plt.gcf().gca()

    if toolRadius != None:    
        #plot circle centered on start point, radius of tool, semi-transparent blue
        ax.add_artist(plt.Circle(segList[0].asXYPairPair()[0], toolRadius, color=(0,0,1,.2)))
    
    ax.set_aspect('equal', adjustable='box')
    # TODO - how to check the current status
    plt.title("GCode ??? layer ??? display, print order blue to red, blue-circle is tool")
    plt.show()


#intercepts a stateful iterator, displays, and returns a stateful one
def displayAndIterateLayer(iterable, toolRadius=None):
    if type(iterable) != list:
        iterable = list(iterable)
    
    displayLayer(iterable, toolRadius)

    def f():
        for s in iterable:
            yield s
    return f()

# TODO - these test could possibly be better
if __name__ == "__main__":
    from testUtils import getFirstGCodePath
    from gcodeParser import *
    
    firstPath = getFirstGCodePath()

    parser = gcodePathParser()
    parseResult = parser.parseFile(firstPath)

    print("Parse finished")

    for layer in parseResult.getLayerGeneratorGenerator():
        displayLayer(layer, 25)