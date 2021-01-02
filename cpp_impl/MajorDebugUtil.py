"""Used for debugging when there is some fundamental problem"""
# such as when your 100% deterministic algorithm is running probabilistically

import itertools
import subprocess

ITEM_TO_RUN = ["Main.exe"]
NUM_TIMES_TO_RUN = 5

NUM_CHARS_PER_HALF = 80
COLUMN_DELIMITER = " | "
ERROR_COLUMN_DELIMITER = " E "


def padStringToLenCenter(baseStr, paddingPattern, targetLen):
    if(len(baseStr) > targetLen):
        raise ValueError("Base str langer than target")

    frontPadding = []
    leadingPadding = []
    backPadding = []
    

    amountToPad = targetLen - len(baseStr)
    timesToPad = amountToPad // len(paddingPattern)
    spacesToAdd = amountToPad % len(paddingPattern)

    if(timesToPad % 2 == 1):
        timesToPad -= 1
        spacesToAdd += len(paddingPattern)
    
    while(spacesToAdd > 0):
        if spacesToAdd == 1:
            backPadding.append(" ")
            spacesToAdd -= 1
        else:
            leadingPadding.append(" ")
            backPadding.append(" ")
            spacesToAdd -= 2

    while (timesToPad > 0):
        frontPadding.append(paddingPattern)
        backPadding.append(paddingPattern)
        timesToPad -= 2

    leadingPadding.append(baseStr)

    finalStr = "".join(itertools.chain(frontPadding, leadingPadding, backPadding))

    assert(len(finalStr) == targetLen)
    return finalStr

if __name__ == "__main__":
    r = subprocess.run(["make", "CPPFLAGS=\"-DDEBUG_4\"", "force"])
    if(r.returncode != 0):
        print("Make failed, terminating")
        exit(1)
    
    print("-----------------------------------")
    print("Build competed ok.")

    runResults = []
    for _ in range(NUM_TIMES_TO_RUN):
        runResults.append(subprocess.run(ITEM_TO_RUN, capture_output=True))
    
    exitCodes  = list(map(lambda x: x.returncode, runResults))

    print("Trial exit codes: ", end="")
    print(exitCodes)

    if(0 not in exitCodes):
        print("No trial finished successfully")
        exit(1)

    firstMismatchTrials = None
    for i in range(0, NUM_TIMES_TO_RUN):
        for j in range(i+1, NUM_TIMES_TO_RUN):
            if(runResults[i].stdout != runResults[j].stdout):
                print(f"Found Output Mismatch trial {i} and {j}")
                firstMismatchTrials = (i,j)
                break
    if(firstMismatchTrials == None):
        print("All trials matched in output, perhaps you need to increase the number of trials")

    i,j = firstMismatchTrials
    linesI = str(runResults[i].stdout.decode("ASCII")).split("\r\n")
    linesJ = str(runResults[j].stdout.decode("ASCII")).split("\r\n")
    maxLen = max(len(linesI), len(linesJ))

    misMatchedLines = []
    # now we want to find all lines where there is an error:
    for i in range(maxLen):
        try:
            if linesI[i] != linesJ[i]:
                misMatchedLines.append(i)
        except IndexError:
            misMatchedLines.append(i)

    print("Lines with mis-match: ", end="")
    # print(misMatchedLines)

    # how many characters in the numbers part
    numbersLength = len(str(maxLen))
    emptyNumbers = " "*numbersLength;
    
    col1Header = padStringToLenCenter(f" i = {firstMismatchTrials[0]}, numLines = {len(linesI)} ", "-", NUM_CHARS_PER_HALF)
    col2Header = padStringToLenCenter(f" j = {firstMismatchTrials[1]}, numLines = {len(linesJ)} ", "-", NUM_CHARS_PER_HALF)


    print(f"{emptyNumbers}{COLUMN_DELIMITER}{col1Header}{COLUMN_DELIMITER}{col2Header}")

    startPoint = max(misMatchedLines[0]-10, 0) # start near first error
    startPoint = 0 # see all

    for i in range(startPoint, maxLen):
        if i > 10+misMatchedLines[0]:
            print("Truncating for ease of reading...")
            break
        
        try:
            strI = linesI[i].replace('\t', "  ")
        except IndexError:
            strI = ""
        
        try:
            strJ = linesJ[i].replace("\t", "  ")
        except IndexError:
            strJ = ""

        thisNumber = str(i)
        thisNumber = (" "*(numbersLength-len(thisNumber)))+thisNumber
        
        

        # figure out what is the longer string's length
        maxLineLen = max(len(strI), len(strJ))
        if(maxLineLen % NUM_CHARS_PER_HALF != 0):
            maxLineLen = (maxLineLen // NUM_CHARS_PER_HALF + 1) * NUM_CHARS_PER_HALF

        # pad both of strings to that length
        strI = strI + " "*(maxLineLen - len(strI))
        strJ = strJ + " "*(maxLineLen - len(strJ))
        
        # print(f"{i} {thisNumber} {maxLineLen}")

        # now we do the printing
        for j in range(maxLineLen // NUM_CHARS_PER_HALF):
            hereNumbers = thisNumber if j == 0 else emptyNumbers
            hereDelimiter = ERROR_COLUMN_DELIMITER if (strI != strJ) else COLUMN_DELIMITER
            partialI = strI[NUM_CHARS_PER_HALF*j:NUM_CHARS_PER_HALF*(j+1)]
            partialJ = strJ[NUM_CHARS_PER_HALF*j:NUM_CHARS_PER_HALF*(j+1)]
            assert(len(partialI) == len(partialJ))
            print(f'{hereNumbers}{hereDelimiter}{partialI}{hereDelimiter}{partialJ}')

    
    # print(padStringToLenCenter(" testy ", "*", 20))


    #print(runResults[0].stdout)
