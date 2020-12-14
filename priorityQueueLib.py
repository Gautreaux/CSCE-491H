
import heapq
from typing import Callable

# TODO - how to handle custom comparator?
#   probably a wrapper class?
class PriorityQueue:
    def __init__(self, comparator: Callable[[], int] = None) -> None:
        if comparator is not None:
            raise NotImplementedError()
        self.heap = []
    
    def push(self, item) -> None:
        heapq.heappush(self.heap, item)
    
    def peek(self) -> not None:
        if len(self) <= 0:
            raise IndexError
        return self.heap[0]
    
    def pop(self) -> not None:
        # heappop will raise an index error if needed
        return heapq.heappop(self.heap)

    def updateFront(self) -> None:
        'Pop the front element and reinsert it to update the costs'
        self.push(self.pop())
    
    def __len__(self) -> int:
        return len(self.heap)
