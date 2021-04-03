
import re

numberSegments = 1237

type0Str = "Starting index: ([\d]+)"
type1Str = "CALC: ([\d]+) ([\d]+)"
type2Str = "CUDA set: ([\d]+) (-?[\d]+)"
type3Str = "HOST read: ([\d]+) (-?[\d]+)"

with open("tempout.txt", 'r') as infile:
    shouldHave = set(range(numberSegments))
    shouldPairs = set([(x,y) for x in shouldHave for y in shouldHave])

    for i in range(numberSegments):
        assert(i in shouldHave)
        for j in range(numberSegments):
            assert((i,j) in shouldPairs)

    assert(len(shouldPairs) == len(shouldHave)**2)

    myHave = set()
    myHave1 = set()
    myHave2 = set()
    myPairs = set()
    myPairs2 = set()

    for line in infile:
        line = line.strip()
        result = re.search(type0Str, line)
        if(result != None):
            i = int(result[1])
            # print(i)
            assert(i not in myHave)
            myHave.add(i)

        # result = re.search(type1Str, line)
        # if(result != None):
        #     i = (int(result[1]), int(result[2]))
        #     assert(i not in myPairs)
        #     myPairs.add(i)

        result = re.search(type2Str, line)
        if(result != None):
            i = int(result[1])
            # print(i)
            assert(i not in myHave1)
            myHave1.add(i)
            p = (i, int(result[2]))
            myPairs.add(p)

        result = re.search(type3Str, line)
        if(result != None):
            i = int(result[1])
            assert(i not in myHave2)
            myHave2.add(i)
            p = (i, int(result[2]))
            myPairs2.add(p)


    print(myPairs.symmetric_difference(myPairs2))
    
    print(len(shouldHave.symmetric_difference(myHave)))
    # print(len(shouldPairs.symmetric_difference(myPairs)))
    print(len(myHave1.symmetric_difference(myHave2)))
    print(len(myPairs.symmetric_difference(myPairs2)))
