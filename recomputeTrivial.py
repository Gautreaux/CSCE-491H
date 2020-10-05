# the trivial recompute framework
# useful for testing

from recomputeFramework import GenericRecompute, NoValidRedefinedPath


class TrivialRecompute(GenericRecompute):
    def recomputePath(self, pathRepr):
        return 1.0
    
    def __str__(self):
        return "Always returns 1"

class InvalidRecompute(GenericRecompute):
    def recomputePath(self, pathRepr):
        raise NoValidRedefinedPath("Always throws this error")

    def __str__(self):
        return "Always throws path"