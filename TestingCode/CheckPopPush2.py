# compares the pops and pushes for the file

from os import stat
import re

from CheckPopPush import CURRENT_REGEX, PUSHING_REGEX, TARGET_FILE, CAPTURE_GROUPS


OUTPUT_FILE = "TestingCode/PQTest2.cpp"

OUT_HEADER = """
#include "../pch.h"\n
#include "../UtilLib/DynamicBitset.h"
#include "../RecomputeState.h"
#include "../PrunedAStar.h"\n

int main(int argc, char** argv){
\tState_PQ pq;
\tRecomputeState poppedState;\n"""

OUT_TAIL = """
\tassert(pq.size() == 0);
\tprintf("Exited normally\\\n");
\treturn 0;
}"""

if __name__ == "__main__":

    print(f"Checking output file: {TARGET_FILE}")
    print(f"Target Output file: {OUTPUT_FILE}")

    statesCounter = 0
    with open(OUTPUT_FILE, 'w') as outFile:
        print(OUT_HEADER, file=outFile)
        with open(TARGET_FILE, 'r') as f:
            for line in f:
                m = re.match(PUSHING_REGEX, line)
                if(m != None):
                    thisState = (tuple(map(lambda x: int(m.group(x), 16), CAPTURE_GROUPS)))
                    print(f"\t//{line.strip()}", file=outFile)
                    thisDBS = f"dbs{statesCounter}"
                    print(f"\tDynamicBitset {thisDBS}(8);", file=outFile)
                    for i in range(8):
                        isBitSet = ((thisState[4] & (1 << i)) != 0)
                        if(isBitSet):
                            print(f"\t{thisDBS}.set({i});", file=outFile)
                    print(f"\tRecomputeState rs{statesCounter}({thisState[0]}, {thisState[1]}, {thisState[2]}, dbs{statesCounter}, nullptr);", file=outFile)
                    print(f"\tstd::cout << \"Pushing new state \" << rs{statesCounter} << std::endl;", file=outFile)
                    print(f"\tpq.push(rs{statesCounter});", file=outFile)
                    print("", file=outFile) # extra newline
                    statesCounter += 1
                    continue
                m = re.match(CURRENT_REGEX, line)
                if(m != None):
                    thisState = (tuple(map(lambda x: int(m.group(x), 16), CAPTURE_GROUPS)))
                    print(f"\t//{line.strip()}", file=outFile)
                    print(f"\tpoppedState = pq.top();", file=outFile)
                    print(f"\tstd::cout << \"Current state: \" << poppedState << std::endl;", file=outFile)
                    print(f"\tpq.pop();", file=outFile)
                    print("", file=outFile) # extra newline
                    continue
        print(OUT_TAIL, file=outFile)