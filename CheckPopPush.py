# compares the pops and pushes for the file

from collections import Counter
from copy import deepcopy
import re
import sys

STATE_REGEX = "A1: ([0-9]), A2: ([0-9]), Depth: ([0-9]), Unset: ([0-9]), BitData: 0x([0-9A-F]{2})"
PUSHING_REGEX = f"Pushing new state {STATE_REGEX}"
CURRENT_REGEX = f"Current state: {STATE_REGEX}"
EXPANDING_REGEX = f"Expanding state {STATE_REGEX}"

CAPTURE_GROUPS = [1,2,3,4,5]

TARGET_FILE = "test.txt"

if __name__ == "__main__":
    if(len(sys.argv) > 1):
        TARGET_FILE = sys.argv[1]
    print(f"Checking output file: {TARGET_FILE}")

    pushedStates = []
    poppedStates = []
    expandedState = []
    
    with open(TARGET_FILE, 'r') as f:
        for line in f:
            m = re.match(PUSHING_REGEX, line)
            if(m != None):
                pushedStates.append(tuple(map(lambda x: int(m.group(x), 16), CAPTURE_GROUPS)))
                continue
            m = re.match(CURRENT_REGEX, line)
            if(m != None):
                poppedStates.append(tuple(map(lambda x: int(m.group(x), 16), CAPTURE_GROUPS)))
                continue
            m = re.match(EXPANDING_REGEX, line)
            if(m != None):
                expandedState.append(tuple(map(lambda x: int(m.group(x), 16), CAPTURE_GROUPS)))
                continue

    print(f"Total pushes: {len(pushedStates)}")
    print(f"Total pops: {len(poppedStates)}")

    assert(len(pushedStates) > 0)
    assert(len(poppedStates) > 0)

    pushCounter = Counter(pushedStates)
    popCounter = Counter(poppedStates)
    expandCounter = Counter(expandedState)

    # these are for the items in test state
    print((0,2,2,4) in pushCounter)
    print((0,2,2,4) in popCounter)

    diffCounter = deepcopy(pushCounter)

    diffCounter.subtract(popCounter)

    print("Pushes vs pop, (+ = more push than pop), (- = more pop than push):")
    print(diffCounter)

    qtyPushNoPop = 0
    qtyPopNoPush = 0
    qtyMultipleExpanded = 0

    print("Pushes with no corresponding pops:")
    for key in pushCounter.keys():
        if key not in popCounter.keys():
            qtyPushNoPop += 1
            print(key, end=", ")
    print("") # newline

    print("Pops with no corresponding pushes:")
    for key in popCounter.keys():
        if key not in pushCounter.keys():
            qtyPopNoPush += 1
            print(key, end=", ")
    print("") # newline

    print("States Expanded multiple times:")
    for key, value in expandCounter.items():
        if(value > 1):
            qtyMultipleExpanded += 1
            print(f"{key}:{value}, ", end="")
    print("") # newline

    print(f"Total qtyPushNoPop: {qtyPushNoPop}")
    print(f"Total qtyPopNoPush: {qtyPopNoPush}")
    print(f"qtyMultipleExpanded: {qtyMultipleExpanded}")
    
    assert(qtyPopNoPush == 0)
    assert(qtyPushNoPop == 0)
    assert(qtyMultipleExpanded == 0)

    print("All checks passed successfully")