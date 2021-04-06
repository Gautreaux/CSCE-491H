
from argparse import ArgumentParser
import os
DEFAULT_LOG_FILE_NAME = "batch.log"
FAILED_FILES_FILE_NAME = 'failedFiles.csv'
PERFORMANCE_STATS_FILE_NAME = "performanceStats.csv"

RECOMPUTE_RES_FILE_NAME = "recomputeRes.csv"

RECOMPUTE_EXTENSION = '.recompute_res'

def processResInDirectory(fDir, outDir, mode):
    fDir = fDir + ('/' if fDir[-1] != '/' else '')
    outDir = outDir + ('/' if outDir[-1] != '/' else '')

    print(f"Processing {RECOMPUTE_EXTENSION} files in the directory {fDir}")

    # print(list(os.listdir(fDir))[0])

    filesList = [f for f in os.listdir(fDir) if
        f[-len(RECOMPUTE_EXTENSION):] == RECOMPUTE_EXTENSION and
        len(f.split(".")) == 3 and
        int(f.split(".")[1]) == mode]

    print(f"Found {len(filesList)} files to process.")

    assert(len(filesList) > 0)

    t = RECOMPUTE_RES_FILE_NAME.split(".")
    outFilePath = outDir + "".join([t[0], '.', str(mode), '.', t[1]])

    print(f"Writing results to {outFilePath}")

    processedFiles = 0
    totalErrorFiles = 0
    totalSeconds = 0

    with open(outFilePath, 'w') as outFile:
        headers = ["file_name", "runtime_error", "parse_error", "recompute_duration", "layers_qty", "new_time_total", "base_time_total", "raw_time_total", "new_print_total"]
        print(str(headers)[1:-1], file=outFile)

        for f in filesList:
            fullPath = fDir + f
            with open(fullPath, 'r') as inFile:
                outList = [f, True, False]
                hasMatch = False

                lineIter = iter(inFile)

                while True:
                    try:
                        l = next(lineIter)
                    except StopIteration:
                        break
                    if("Recompute duration (s): " in l):
                        assert(hasMatch == False)
                        assert(outList[2] == False)
                        hasMatch = True
                        outList.append(float(l.split(" ")[3]))
                        totalSeconds += outList[-1]

                        l = next(lineIter)
                        assert("layers" in l)
                        outList.append(int(l.split(" ")[1]))

                        l = next(lineIter)
                        assert("Total new time:" in l)
                        outList.append(int(l.split(" ")[3]))

                        l = next(lineIter)
                        assert("Total base time:" in l)
                        outList.append(int(l.split(" ")[3]))

                        l = next(lineIter)
                        assert("Total raw print time:" in l)
                        outList.append(int(l.split(" ")[4]))

                        l = next(lineIter)
                        assert("Total new print time: " in l)
                        outList.append(int(l.split(" ")[4]))

                    elif("gcp failed, exiting" in l):
                        assert(hasMatch == False)
                        assert(outList[2] == False)
                        assert(len(outList) == 3)
                        outList[-1] = True

                        while(len(outList) < len(headers)):
                            outList.append("")

                if len(outList) == len(headers):
                    outList[1] = False
                else:
                    totalErrorFiles += 1

                print(str(outList)[1:-1], file=outFile)

                processedFiles += 1

                if(processedFiles % 250 == 0):
                    print(f"Processed {processedFiles} of {len(filesList)}")

    totalFiles = len(filesList)
    print(f"Total Files: {totalFiles}")
    print(f"Error Files: {totalErrorFiles}")
    print(f"  % {round((totalErrorFiles*100)/totalFiles, 3)}")
    print(f"CPU Time (seconds): {round(totalSeconds,1)}")
    print(f"CPU Time (minutes): {round(totalSeconds/60, 3)}")
    print(f"CPU Time (hours): {round(totalSeconds/3600, 3)}")
    print(f"CPU Time (days): {round(totalSeconds/(24*3600), 3)}")

def processLogFileInDirectory(fDir, name, mode):
    if(name[-len('.log'):] != ".log"):
        print(f"Appending '.log' to provided filename: '{name}'")
        name += '.log'

    fDir = fDir + ('/' if fDir[-1] != '/' else '')
    
    print(f"Processing log file '{name}' in {fDir}")

    fullPath = fDir + name
    failedFilesPath = fDir + FAILED_FILES_FILE_NAME.replace(".csv", ".") + str(mode) + ".csv"
    statsPath = fDir + PERFORMANCE_STATS_FILE_NAME.replace(".csv", ".") + str(mode) + ".csv"

    failedFilesCount = 0

    headers = ["seconds", "completed_qty", "remaining_qty", "mb_processed"]
    ffHeaders = ["name", "errorCode"]

    with open(fullPath, 'r') as inFile:
        lineIter = iter(inFile)
        with open(failedFilesPath, 'w') as failedFiles:
            with open(statsPath, 'w') as statsFile:
                print(str(headers)[1:-1], file=statsFile)
                print(str(ffHeaders)[1:-1], file=failedFiles)

                l = next(lineIter).strip()
                assert("Running with" in l)
                print(l)
                l = next(lineIter).strip()
                assert("Core count is " in l)
                print(l)
                l = next(lineIter).strip()
                assert("Running in mode" in l)
                print(l)
                m_log = int(l.split(" ")[3])
                assert(m_log == mode)
                l = next(lineIter).strip()
                assert(" files in directory." in l)
                print(l)
                totalFiles = int(l.split(" ")[1])

                while True:
                    try:
                        l = next(lineIter).strip()
                    except StopIteration:
                        break
                    if("Approx Processing Rate " in l):
                        continue
                    elif("Error while processing file " in l):
                        t = l.split(" ")
                        fName = t[4].split('/')[3]
                        returnCode = t[-1]
                        int(returnCode) # just ensure we can convert to int
                        print(f"{fName}, {returnCode}", file=failedFiles)
                        failedFilesCount += 1
                        continue
                    elif("Running with " in l):
                        l1 = next(lineIter).strip()
                        l2 = next(lineIter).strip()

                        completed = int(l.split(" ")[2])
                        remaining = int(l.split(" ")[4])
                        mb = float(l1.split(" ")[1])
                        time = int(l2.split(" ")[2][:-1])

                        print(str([time, completed, remaining, mb])[1:-1], file=statsFile)
                    else:
                        raise RuntimeError(f"Unknown line: '{l}'")
    print(f"Total Files: {totalFiles}")
    print(f"Failed qty: {failedFilesCount}")
    print(f"  %: {round((failedFilesCount*100)/totalFiles, 3)}")
    print(f"Wall Time (seconds): {time}")
    print(f"Wall Time (minutes): {round(time/60, 3)}")
    print(f"Wall Time (hours): {round(time/3600, 3)}")
    print(f"Wall Time (days): {round(time/(24*3600), 3)}")

if __name__ == "__main__":
    parser = ArgumentParser(description="Process the output data from the batchRunner.")
    parser.add_argument('-d', nargs=1, help=f"Process '{RECOMPUTE_EXTENSION}' files in the specified directory.")
    parser.add_argument('-o', nargs=1, help="Directory for output files and of input log file, if no value provided, use the current working directory", default=[os.getcwd()])
    parser.add_argument('-n', nargs=1, help=f"Name of the .log file, default is '{DEFAULT_LOG_FILE_NAME}'", default=[DEFAULT_LOG_FILE_NAME])
    parser.add_argument('-m', nargs=1, help="Which mode of output to process. 0,1,2,3?")

    args = parser.parse_args()

    args.m = int(args.m[0])
    assert(args.m in range(5))

    if(args.d is not None):
        processResInDirectory(args.d[0], args.o[0], args.m)
    else:
        processLogFileInDirectory(args.o[0], args.n[0], args.m)