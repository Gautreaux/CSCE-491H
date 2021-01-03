# the two fundamental recompute data
# the sum of all print-segment times
# the sum of all print+travel segment times
# these both say time, but in reality they are distance
#   but by assigning a speed, distance is functionally time
#   so i'm rolling with this for now

from recomputeFramework import GenericRecompute
from gcodeParser import ParseResult

class PrintTimeRecompute(GenericRecompute):
    def __init__(self):
        self.totalTime = 0

    def recomputeLayer(self, layerGenerator):
        return sum(map(lambda x: x.length() if x.isPrint() else 0, layerGenerator))

    def recomputePath(self, pathRepr):
        if(type(pathRepr) != ParseResult):
            raise TypeError("Cannot recompute on this repr, expected ParseResult type")
        layerGeneratorGenerator = pathRepr.layerGeneratorGenerator()
        return sum(map(self.recomputeLayer, layerGeneratorGenerator))
    
    def __str__(self):
        return "Total Print segment time"

# TODO - should both travel and non-travel time cost the same?
class PrintTravelTimeRecompute(GenericRecompute):
    def __init__(self):
        self.totalTime = 0

    def recomputeLayer(self, layerGenerator):
        return sum(map(lambda x: x.length(), layerGenerator))

    def recomputePath(self, pathRepr):
        if(type(pathRepr) != ParseResult):
            raise TypeError("Cannot recompute on this repr, expected ParseResult type")
        layerGeneratorGenerator = pathRepr.layerGeneratorGenerator()
        return sum(map(self.recomputeLayer, layerGeneratorGenerator))
    
    def __str__(self):
        return "Total print and travel time"