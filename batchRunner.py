
from argparse import ArgumentParser
from time import sleep, time
from multiprocessing import cpu_count
import subprocess
import os



if __name__ == "__main__":
    parser = ArgumentParser(description="Process a series of files, logging output to text files.")
    parser.add_argument('-d', nargs=1, help="run on the specified directory")
    parser.add_argument('-f', nargs=1, help="run on the specified file")
    parser.add_argument('-t', nargs=1, help="number of threads to spawn, pass negative number for native cores - argument", default=cpu_count())
    parser.add_argument('-o', nargs=1, help="Output directory, if not present, use -d value", default=None)
    args = parser.parse_args()
    
    if(args.d is None and args.f is None):
        print("Error, must specify one of -d <dirname>, -f <filename>")
        exit(1)
    if(args.d is not None and args.f is not None):
        print("Error, must specify exactly one of -d <dirname>, -f <filename>")
        exit(1)
    
    if(args.f is not None):
        raise NotImplementedError("Running on specific file is not implemented yet.")

    if(type(args.t) == list):
        # a command line argument was provided
        args.t = int(args.t[0])

    if(args.t < 0):
        args.t = cpu_count() + args.t

    if(args.t > cpu_count()):
        print(f"Warning, provided thread count ({args.t}) exceeds native core count ({cpu_count()})")
        print("This is unlikely to provide performance improvement, and may negatively impact performance")
        while True:
            print("Continue? (Y/N): ")
            c = input()
            if c[0].upper() == 'Y':
                break
            if c[0].upper() == 'N':
                exit(0)

    if(args.o is None):
        args.o = args.d
    
    if(os.path.isdir(args.d[0]) is False):
        print("Error: provided directory is not a directory")
        exit(1) 

    if(os.path.isdir(args.o[0]) is False):
        print("Error: provided output directory is not directory")
        exit(1)        
        
    print(f"Running with {args.t} threads.")
    print(f"Core count is {cpu_count()}.")

    filesList = [f for f in os.listdir(args.d[0]) if
            f[-len('.gcode'):] == ".gcode"]

    print(f"Found {len(filesList)} files in directory.")

    processes = [None]*args.t
    pendingSizes = [None]*args.t
    filePaths = [None]*args.t
    fileHandles = [None]*args.t

    def filesGenerator():
        for f in filesList:
            yield f
        
    filesToProcess = len(filesList)
    filesIter = filesGenerator()

    filesCompleted = 0
    bytesCompleted = 0

    startTime = time()

    while filesToProcess > 0:
        for i in range(len(processes)):
            if(processes[i] != None):
                if(processes[i].poll() != None):
                    #this process has finished

                    #TODO - stats

                    # print(f"Finished {fpath}")

                    filesToProcess -= 1
                    filesCompleted += 1
                    bytesCompleted += pendingSizes[i]

                    if processes[i].returncode != 0:
                        print(f"Error while processing file {filePaths[i]}, returned {processes[i].returncode}")

                    processes[i] = None
                    pendingSizes[i] = None
                    filePaths[i] = None
                    fileHandles[i].close()
                    fileHandles[i] = None
            try:
                if(processes[i] == None):
                    fpath = next(filesIter)
                    outPath = args.o[0] + fpath[:-len('.gcode')] + '.recompute_res'
                    fpath = args.d[0] + fpath
                    # print(f"Starting {fpath}->{outPath}")
                    pendingSizes[i] = os.stat(fpath).st_size
                    filePaths[i] = fpath
                    fileHandles[i] = open(outPath, 'w')
                    processes[i] = subprocess.Popen(["./Main", fpath], stdout=fileHandles[i])
            except StopIteration:
                pass

        print(f"Running with {filesCompleted} completed, {filesToProcess} remaining.")
        mb = round(bytesCompleted/(1000*1000), 3)
        es = round(time()-startTime)
        print(f"\tProcessed {mb} MB")
        print(f"\tElapsed time {es}s")
        print(f"Approx Processing Rate {round(mb/es, 3)} MB/s")
        sleep(5)