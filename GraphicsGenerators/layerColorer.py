
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors

from GraphicsGenerators.gcodeParser import GCodeSegment, ParseResult
from typing import List

def colorLayer(parseResult : ParseResult, name = ''):
    for layerGenerator in parseResult.layerGeneratorGenerator():
        layerSegs : List[GCodeSegment] = list(layerGenerator)
        print(f"{len(layerSegs)} z={layerSegs[0].startPos[2]}")

        zLayer = layerSegs[0].startPos[2]

        lines = list(map(lambda x: ((x.startPos[0], x.endPos[0]), (x.startPos[1], x.endPos[1])), layerSegs))
        colorsAndLabels = list(map(lambda x: ('b','print segment') if x.isPrint() else ('g', 'travel segment'), layerSegs))
        

        plt.figure()
        legendSet = set()
        for ((x,y),(c,l)) in zip(lines, colorsAndLabels):
            # print(f"{x} {y} {c}")
            if(l in legendSet):
                plt.plot(x,y,color=c, label='_')
            else:
                plt.plot(x,y,color=c, label=l)
                legendSet.add(l)
        ax = plt.gcf().gca()
        ax.legend(loc=0)
        ax.set_aspect('equal', adjustable='box')

        # Shrink current axis by 20%
        box = ax.get_position()
        ax.set_position([box.x0, box.y0, box.width * 0.8, box.height * 0.8])

        # Put a legend to the right of the current axis
        ax.legend(loc='center left', bbox_to_anchor=(1, 0.5))

        plt.title(f"{name}    z = {zLayer}")
        plt.xlabel("X (mm)")
        plt.ylabel("Y (mm)")
        plt.show()


def colorChains(parseResult : ParseResult, name = ''):
    for layerGenerator in parseResult.layerGeneratorGenerator():
        layerSegs : List[GCodeSegment] = list(layerGenerator)
        print(f"{len(layerSegs)} z={layerSegs[0].startPos[2]}")

        printSegs = [seg for seg in layerSegs if seg.isPrint()]

        zLayer = layerSegs[0].startPos[2]

        lines = list(map(lambda x: ((x.startPos[0], x.endPos[0]), (x.startPos[1], x.endPos[1])), printSegs))
        
        colors = []
        inChain = False
        thisColor = None
        def colorGenerator():
            while(True):
                for e in mcolors.TABLEAU_COLORS:
                    yield e

        colorGen = colorGenerator()
        
        for seg in layerSegs:
            if seg.isPrint() is False:
                inChain = False
            else:
                if inChain is False:
                    thisColor = next(colorGen)
                    inChain = True
                colors.append(thisColor)

        assert(len(colors) == len(lines))
        

        plt.figure()
        for ((x,y),c) in zip(lines, colors):
            # print(f"{x} {y} {c}")
            plt.plot(x,y,color=c)
        ax = plt.gcf().gca()

        plt.title(f"{name}    z = {zLayer}")
        
        ax.set_aspect('equal', adjustable='box')

        plt.show()