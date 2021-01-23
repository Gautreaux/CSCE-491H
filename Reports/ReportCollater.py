
import os
import sys
from typing import Any, Generator, List, Tuple

EXT_TYPE = ".repOut"

#convert a list to a comma seperated value representation
def convertListToCSVstr(inList : List[Any]) -> str:
    if(len(inList) == 0):
        return ""

    outList = [inList[0]]
    for i in range(1, len(inList)):
        outList.append(',')
        outList.append(inList[i])
    
    return "".join(outList)

def filesList(rootName : str) -> List[str]:
    # print(os.listdir())
    return [f for f in os.listdir() if ((f.find(rootName) == 0) and (f.find(EXT_TYPE) != -1))]

# generate lists of the report instances
def reportInstanceGenerator(sourceFilePath) -> Generator[List[str], None, None]:
    with open(sourceFilePath, 'r') as inFile:
        f = []
        for line in inFile:
            line = line.strip()
            if line == "":
                yield f
                f = []
            else:
                f.append(line)
        if(f != []):
            yield f
            f = []
    

def reportInstanceGeneratorNoNewline(sourceFilePath) -> Generator[List[str], None, None]:
    with open(sourceFilePath, 'r') as inFile:
        lineIter = iter(inFile)

        f = None

        try:
            while(True):
                f = [next(inFile).strip()]
                while(True):
                    n = next(inFile)
                    if '\t' in n:
                        f.append(n.strip())
                    else:
                        # start a new file
                        yield f
                        f = [n.strip()]
        except StopIteration:
            if f != []:
                yield f

def zLayerGenerator(lines, startInd = 4) -> Generator[Tuple[str], None, None]:
    assert((len(lines) - startInd) % 3 == 0)
    for i in range(startInd, len(lines), 3):
        yield (lines[i], lines[i+1], lines[i+2])

class BreakContinue(Exception):
    pass

# process the fileList
def processSegmentsGenericReports(outFileName, sourceFilesList : List[str]):
    # print the headings for columns we want to print
    headers = ["file_path", "report_file_path", "file_size_bytes", 
                "is_valid", "parse_error", "mono_error", "z_error", 
                "continuous_error", "too_large_error", 
                "num_orig_segments", "num_segments", "num_z_layers",
                "largest_layer_num_all_seg", "largest_layer_all_seg_z",
                "largest_layer_num_print_seg", "largest_layer_print_seg_z", 
                "largest_layer_print_dist", "larget_layer_print_dist_z"]

    with open(outFileName, 'w') as outFile:
        print(convertListToCSVstr(headers), file=outFile)
        # process each shard
        for sourceFileName in sourceFilesList:
            print(f"Starting file {sourceFileName}")

            for reportInstance in reportInstanceGenerator(sourceFileName):
                fields = []

                # check if an exception occurred
                try:
                    for c in reportInstance:
                        if "EXCEPTION" in c:
                            print(f"Exception on {fields[0]}")
                            raise BreakContinue
                except BreakContinue:
                    continue

                # log path (one guaranteed)
                fields.append(reportInstance[0])
                # log source file path
                fields.append(sourceFileName)

                # log filesize
                fields.append(reportInstance[1])

                isValid = ("VALID" in reportInstance[2])
                fields.append("TRUE" if isValid else "FALSE")

                if(not isValid):
                    s = reportInstance[2].split(" ")[1:]
                    assert(len(s) == 5)
                    # log parse_error
                    fields.append("TRUE" if s[0] == '1' else "FALSE")
                    # log mono_error
                    fields.append("TRUE" if s[1] == '1' else "FALSE")
                    # log z_error
                    fields.append("TRUE" if s[2] == '1' else "FALSE")
                    # log continuous_error
                    fields.append("TRUE" if s[3] == '1' else "FALSE")
                    # log too_large_error
                    fields.append("TRUE" if s[4] == '1' else "FALSE")
                    pass
                else:
                    # log parse_error,  mono_error, z_error, log continuous_error, log too_large_error
                    for _ in range(5):
                        fields.append("FALSE")

                    # log segments counts
                    s = reportInstance[3].split(" ")
                    assert(len(s) == 3)
                    for e in s:
                        fields.append(e)

                    # get the largest_layer_num_segments and the z value
                    # "largest_layer_num_all_seg", "largest_layer_all_seg_z",
                    # "largest_layer_num_print_seg", "largest_layer_print_seg_z", 
                    # "largest_layer_print_dist", "larget_layer_print_dist_z"
                    llnas = 0
                    llasz = ''
                    llnps = 0
                    llnpz = ''
                    llpd = 0
                    llpdz = ''
                    for zLayer in zLayerGenerator(reportInstance):
                        t = zLayer[0]
                        t = t.split(" ")
                        assert(len(t) == 3)
                        z = t[0]
                        i = int(t[1])
                        if(i > llnas):
                            llnas = i
                            llasz = z

                        t = zLayer[1]
                        t = t.split(" ")
                        assert(len(t) == 4)
                        i = int(t[0])
                        if(i > llnps):
                            llnps = i
                            llnpz = z
                        i = round(float(t[2]), 3)
                        if(i > llpd):
                            llpd = i
                            llpdz = z

                    fields.append(str(llnas))
                    fields.append(llasz)
                    fields.append(str(llnps))
                    fields.append(llnpz)
                    fields.append(str(llpd))
                    fields.append(llpdz)
                        
                print(convertListToCSVstr(fields), file=outFile)

def processPointsReport(outFileName, sourceFilesList: List[str]):
    headers = ['file_path', 'biggest_num_pts', 'biggest_num_pts_z']

    with open(outFileName, 'w') as outFile:
        print(convertListToCSVstr(headers), file=outFile)

        for sourceFileName in sourceFilesList:
            print(f"Starting file {sourceFileName}")

            for reportInstance in reportInstanceGeneratorNoNewline(sourceFileName):
                fields = []

                # check if an exception occurred
                try:
                    for c in reportInstance:
                        if "EXCEPTION" in c:
                            print(f"Exception on {fields[0]}")
                            raise BreakContinue
                except BreakContinue:
                    continue

                # log filepath
                fields.append(reportInstance[0])

                maxP = 0
                maxP_Z = "None"

                for i in range(1, len(reportInstance)):
                    Z,P_count = reportInstance[i].split(' ')
                    P_count = int(P_count)
                    if(P_count > maxP):
                        maxP = P_count
                        maxP_Z = Z

                fields.append(str(maxP))
                fields.append(maxP_Z)

                print(convertListToCSVstr(fields), file=outFile)


REPORTING_TYPES = {
    "GenericReport" : processSegmentsGenericReports,
    "PointsReport" : processPointsReport,
}

if __name__ == "__main__":
    # debug code
    # os.chdir("D:\\12dPrinter\\research-code\\Reports")
    # sys.argv.append("GenericReport")

    if(len(sys.argv) != 2):
        print("Error, expected one argument <root_report_name>")
        exit(1)

    rootName = sys.argv[1]

    print(f"Root name: {rootName}")
    if(rootName not in REPORTING_TYPES):
        print(f"RootName {rootName} not supported. Perhaps you should implement it.")
        exit(1)

    outFileName = f"{rootName}.csv"
    sourceFiles = filesList(rootName)
    if(outFileName in sourceFiles):
        sourceFiles.remove(outFileName)
    assert(outFileName not in sourceFiles)
    assert(len(sourceFiles) > 0)

    REPORTING_TYPES[rootName](outFileName, sourceFiles)
