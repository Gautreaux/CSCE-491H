# compares the pops and pushes for the file


import re
import sys

from CheckPopPush import CURRENT_REGEX, PUSHING_REGEX, TARGET_FILE, CAPTURE_GROUPS

def isRelevantLine(line):
    m = re.match(PUSHING_REGEX, line)
    if(m != None):
        return True
    
    m = re.match(CURRENT_REGEX, line)
    if(m != None):
        return True
    
    return False

def assertNoMoreRelevant(line_gen):
    '''Assert that the line_gen contains no more relevant lines'''
    # i.e. both generators stop at the same point
    for line in line_gen:
        assert(isRelevantLine(line) == False)

if __name__ == "__main__":
    assert(len(sys.argv) == 3)

    path_a = sys.argv[1]
    path_b = sys.argv[2]
    print(f"File A: {path_a}")
    print(f"File B: {path_b}")

    with open(path_a, 'r') as file_a:
        with open(path_b, 'r') as file_b:
            gen_a = iter(file_a)
            gen_b = iter(file_b)

            line_a = ""
            line_b = ""

            while(True):
                # get the next relevant line from file a
                try:
                    while(True):
                        line_a = next(gen_a)
                        if(isRelevantLine(line_a)):
                            break
                except StopIteration:
                    assertNoMoreRelevant(gen_b)
                    break

                try:
                    while(True):
                        line_b = next(gen_b)
                        if(isRelevantLine(line_b)):
                            break
                except StopIteration:
                    assertNoMoreRelevant(gen_a)
                    break

                try:
                    assert(line_a == line_b)
                except AssertionError as e:
                    print(line_a.strip())
                    print(line_b.strip())
                    raise e

