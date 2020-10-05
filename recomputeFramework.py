# the library for working with recomputing gcode

from abc import ABC, abstractmethod


class NoValidRedefinedPath(Exception):
    pass

# all derived classes should be multi-processing ready
class GenericRecompute(ABC):
    def __init__(self):
        pass

    # given an input
    #   return a list ordered
    #       the zero-th element is a single float that represents print time
    #       subsequent elements are implemenetation specific
    # throw NoValidRedefinedPath exception if no path possible
    @abstractmethod
    def recomputePath(self, pathRepr):
        return

    def __str__(self):
        return "GenericRecompute"

# all derived classes should be multi-processing ready
class BatchRecompute(object):
    def __init__(self):
        self.recomputeModels = []  # the formats of recompute
        self.output = {}

    # add a recompute model
    # model should be a class type derived from GenericRecompute
    def addRecomputeModel(self, model):
        self.recomputeModels.append(model())    
    
    def __len__(self):
        return len(self.recomputeModels)
    
    def getModelCount(self):
        return len(self)

    # use to recover the order of recompute
    def getRecomputeGenerator(self):
        for model in self.recomputeModels:
            yield model    

    # process all the objects in iterable on all the recompute models available
    #   constructs/returns a list where each index
    #       corresponds to the order that the recompute have been added
    #       is the result from GenericRecompute.recomputePath
    #           or the exception it threw
    #   list output is available in getCachedOutput()
    def doBatchProcess(self, item, allowCache=True, suppressException=False):
        if self.getModelCount() <= 0:
            raise ValueError("There are 0 recompute models")

        if (id(item) in self.output
                and allowCache is True
                and len(self.output[id(item)]) == len(self)):
            return self.output[id(item)]

        thisResult = []
        for model in self.recomputeModels:
            try:
                k = model.recomputePath(item)
            except NoValidRedefinedPath as e:
                k = e
            except Exception as e:
                if suppressException is True:
                    k = e
                else:
                    raise e
            thisResult.append(k)
        self.output[id(item)] = thisResult

        return thisResult

    # returns the cached version 
    def getCachedOutput(self, item):
        try:
            return self.output[id(item)]
        except KeyError:
            return None
