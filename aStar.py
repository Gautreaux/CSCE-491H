# implements a generic version of A*
from queue import PriorityQueue

# stateGenerator - a function that
#       when given None returns a list of the starting state(s)
#       when given a state returns a list of following state(s)
# isGoalState - a function that given a state
#       returns a bool true iff this is a goal state
# isVisited - a function that given a state
#       returns a bool true iff this state was already visited
#       if the state was not visited, marks the state as visited
# Note: state representation should be total ordering compatable
# returns - first goal state reached
def aStar(stateGenerator, isGoalState, isVisited):
        statesList = []
        queue = PriorityQueue()

        for startingState in stateGenerator(None):
            queue.put((startingState.priority(), startingState))

        i = 0
        while not queue.empty():
            (_, myState) = queue.get()
            queue.task_done()  # probably wrong if ever multi-threaded
            i += 1

            if isVisited(myState):
                continue
            
            if isGoalState(myState):
                # reached the first goal
                return myState
        
            for nextState in stateGenerator(myState):
                queue.put((nextState.priority(), nextState))
        
        raise NoValidPath()

class NoValidPath(Exception):
    pass

# TODO - define a state ADT and integrate more better
