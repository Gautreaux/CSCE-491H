#include "pch.h"

#include <atomic>
#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>

#include "GCodeParser.h"
#include "PrunedAStar.h"
#include "UtilLib/FileUtil.h"
#include "UtilLib/ThreadsafeIntGen.h"

using std::cout;
using std::endl;

#define DEFAULT_FILEPATH "TestingFiles/simpleRecomputable.gcode"

#ifndef DEFAULT_THREAD_COUNT
#define DEFAULT_THREAD_COUNT 8
#endif

#define REPORT_INTERVAL_SEC 2

struct ParsedArgs{
    bool help;
    bool parseError;
    bool isDirectory;
    std::string path_str;

    ParsedArgs() : help(false), parseError(false), 
        isDirectory(false), path_str()
        {}
};

const ParsedArgs parseArgs(const int argc, char ** argv){
    ParsedArgs p;
    
    for(int i = 1; i < argc; i++){
        const char* thisArg = argv[i];
        if(strcmp(thisArg, "-h") == 0){
            p.help = true;
            continue;
        }
        else if(strcmp(thisArg, "-d") == 0){
            p.isDirectory = true;
            continue;
        }

        //not a flag
        // for now it must be a path
        if(p.path_str == ""){
            p.path_str = std::string(thisArg);
            continue;
        }
        
        printf("Unrecognized argument '%s'\n", thisArg);
        p.parseError = true;
    }

    //set any default values
    if(p.path_str == ""){
        if(p.isDirectory == false){
            p.path_str = DEFAULT_FILEPATH;
        }else{
            printf("Explicit path must be provided with -d flag.\n");
            p.parseError = true;
        }
    }

    return p;
}

void printHelp(void){
    printf("TODO: help information\n");
}

void printError(void){
    printf("An error occurred in argument processing, terminating\n");
}

struct CommonThreadParameters{
    ThreadsafeIntGen *const t;
    const std::vector<std::string> *const fileNames;
    const char* const rootName;
    const std::string outFileRoot;

    std::atomic<unsigned int> totalProcessed;
    std::atomic<unsigned int> totalValid;
    std::atomic<unsigned long long> totalBytesValid;
    std::atomic<unsigned int> threadsRunning;
    std::atomic<bool> running;

    CommonThreadParameters(ThreadsafeIntGen* t, 
        std::vector<std::string> *const fileNames,
        const char* const rootName, const std::string outFileRoot = "") : 
        t(t), fileNames(fileNames), rootName(rootName), outFileRoot(outFileRoot),
        totalProcessed(0), totalValid(0), totalBytesValid(0), threadsRunning(0), running(true)
    {}
};

#define PRINT_BOOL(b) (b ? '1' : '0')

class LayerSummer{
private:
    unsigned int numPrint;
    unsigned int numNonPrint;
    unsigned int numSegments;
    double printDist;
    double nonPrintDist;
public:
    LayerSummer() : numPrint(0), numNonPrint(0), numSegments(0),
        printDist(0), nonPrintDist(0)
    {}

    void operator()(const GCodeSegment& seg){
        numSegments += 1;
        if(seg.isPrintSegment()){
            numPrint += 1;
            printDist += seg.length();
        }else{
            numNonPrint += 1;
            nonPrintDist += seg.length();
        }
    }

    void dump(std::ostream& o, std::string prefix = "\t\t"){
        o << prefix << numPrint << " ";
        o << numNonPrint << " ";
        o << printDist << " ";
        o << nonPrintDist;
    }

    inline unsigned int getNumSegments(void) const {return numSegments;}
};

void dumpGCP(std::ostream& o, const GCodeParser& gcp){
    o << '\t' << gcp.getFileSize() << std::endl;
    o << '\t' << (gcp.isValid() ? "VALID" : "INV");
    if(!gcp.isValid()){
        o << " " << PRINT_BOOL(gcp.parseError()) << " " << PRINT_BOOL(gcp.monoError()) << " " << PRINT_BOOL(gcp.zError()) << " " << PRINT_BOOL(gcp.continuousError()) << " " << PRINT_BOOL(gcp.tooLargeError()) << std::endl;
        o << std::endl; //extra newline to separate records
        return;
    }
    o << std::endl; // ends the valid invalid line
    o << '\t' << gcp.numberOrigSegments() << " " << gcp.numberSegments() << " " << gcp.numberZLayers() << std::endl;
    
    const std::vector<double>& layersRef = gcp.getLayerVecRef();
    unsigned int nextStart = 0;
    unsigned int nextOrigStart = 0;

    for(unsigned int i = 0; i < layersRef.size(); i++){
        double layerValue = layersRef.at(i);
        unsigned int thisStart = nextStart;
        unsigned int thisOrigStart = nextOrigStart;

        if(i == (layersRef.size()-1)){
            //this is the last layer
            nextStart = -1;
            nextOrigStart = -1;
        }else{
            nextStart = gcp.getLayerStartIndex(layersRef.at(i+1), thisStart);
            nextOrigStart = gcp.getLayerOrigStartIndex(layersRef.at(i+1), thisOrigStart); 
        }

        unsigned int thisEnd = gcp.getLayerEndIndex(layerValue, nextStart);
        unsigned int thisOrigEnd = gcp.getLayerOrigEndIndex(layerValue, nextOrigStart);

        LayerSummer seg;
        iterateGCPSlice(gcp, thisStart, thisEnd, seg);

        LayerSummer orig;
        iterateOrigGCPSlice(gcp, thisOrigStart, thisOrigEnd, orig);

        o << '\t' << layerValue << " ";
        o << seg.getNumSegments() << " ";
        o << orig.getNumSegments() << std::endl;

        seg.dump(o);
        o << std::endl;
        orig.dump(o);
        o << std::endl;

    }
    o << std::endl;
}

void threadFunction(const unsigned int threadID, CommonThreadParameters *const CTP){
    CTP->threadsRunning.fetch_add(1);
    unsigned int totalProcessed = 0;
    int fileIndex;

    std::string outPath = CTP->outFileRoot + std::to_string(threadID) + ".repOut";
    std::ofstream ofs(outPath.c_str());

    while(((fileIndex = CTP->t->next()) < ((int)CTP->fileNames->size()))){
        totalProcessed += 1;

        std::string fullPath(CTP->rootName);
        fullPath += '/';
        fullPath += CTP->fileNames->at(fileIndex);

        ofs << fullPath << std::endl;
        try {
            GCodeParser gcp(fullPath);

            dumpGCP(ofs, gcp);

            if(gcp){
                CTP->totalValid.fetch_add(1);
                CTP->totalBytesValid.fetch_add(gcp.getFileSize());
            }
        }catch (...){
            ofs << "\tEXCEPTION" << std::endl;
        }

        CTP->totalProcessed.fetch_add(1);
        ofs.flush();
    }

    ofs.close();

    printf("Thread %u processed %u files\n", threadID, totalProcessed);
    CTP->threadsRunning.fetch_sub(1);
    if(CTP->threadsRunning.load() == 0){
        //technically could be run multiple times
        CTP->running.store(false);
    }
}

void reportingFunction(const CommonThreadParameters *const CTP){
    unsigned int approx_sec = 0;
    unsigned int count =  CTP->fileNames->size();
    unsigned long long lastBytes = 0;
    
    while(CTP->running.load() > 0){
        unsigned int total = CTP->totalProcessed.load();    
        unsigned long long bytes = CTP->totalBytesValid.load();
        
        unsigned long long elapsedBytes = (bytes - lastBytes);
        double MBPS = ((double)(elapsedBytes * B_TO_MB))/REPORT_INTERVAL_SEC;

        printf("[%u], Running: %u, Processed %u/%u (%.2f%%), Valid %u, Bytes (valid) %.0fMB (%.2fMB/s)\n", 
            approx_sec, CTP->threadsRunning.load(), total, count, (((double)(total*100))/count),
            CTP->totalValid.load(), bytes * B_TO_MB, MBPS
        );

        //updating fields
        lastBytes = bytes;

        std::this_thread::sleep_for(std::chrono::seconds(REPORT_INTERVAL_SEC));
        approx_sec += REPORT_INTERVAL_SEC;
    }
}

int main(int argc, char ** argv){
    const ParsedArgs p = parseArgs(argc, argv);

    if(p.help){
        printHelp();
        exit(0);
    }else if(p.parseError){
        printError();
        exit(1);
    }

    printf("Arguments Processed successfully: path\n");
    printf("\tPath: %s\n", p.path_str.c_str());
    printf("\tType: %s\n", (p.isDirectory ? "DIR" : "FILE"));

    if(p.isDirectory == false){
        //process just the single file
        GCodeParser gcp(p.path_str);

        //FYI - letting this run with only dual prints.
        //  can get to 344/652 printed on the first layer
        //FYI - letting this run with dual prints and print/noop, 
        //  infinite stepback, setbit priority
        //  389/652 @ 50 million states (~3GB ram)
        //  392/652 @ 85 million states (~5GB ram)
        //  cancelled @ 100 million states (~6GB ram)
        //FYI - letting this run with dual prints and print/noop,
        //  infinite stepback, depthcompare
        //  393/652 @ ~3 million states (~?GB ram)
        //          @ ~50 million states (~3GB ram)
        //          @ ~100 million states (~6GB ram)
        //FYI - letting thus run with dual prints and print/noop,
        //  infinite stepback, prioritycompare
        //  372/652 @ ~1 million states (~?GB ram)
        
        //GCodeParser gcp("gcodeSampleSet/81191.gcode");

        if(gcp){
            printf("Found %d segments in %d layers in the file.\n", gcp.numberSegments(), gcp.numberZLayers());
            if(gcp.numberSegments() < 30){
                for (auto it = gcp.segments_begin(); it != gcp.segments_end(); it++){
                    cout << *it << endl;
                }
                cout << endl;
            }
        }else{
            printf("gcp failed, exiting");
            exit(1);
        }

        prunedAStar(gcp);
        std::cout << "Normal Exit occurred" << std::endl;
    }
    else
    {
        //process a directory
        std::vector<std::string> files;

        auto totalFiles = getAllFiles(p.path_str.c_str(), files, FileFilterExtension(".gcode"));
        assert(files.size() == totalFiles);

        //print all the files
        // for(auto s : files){
        //     printf("%s/%s\n", p.path_str.c_str(), s.c_str());
        // }

        printf("Directory found %u total files\n", totalFiles);

        //number of threads to spawn
        //  should become a command line parameter
        unsigned int numThreads = DEFAULT_THREAD_COUNT;
#ifdef FORCE_SINGLE_THREAD
        numThreads = 1;
#endif
        if(numThreads > 500){
            printf("NumThreads seems extremely high: %u\n", numThreads);
            exit(2);
        }

        // generator for which files to process
        ThreadsafeIntGen gen;

        //create the thread parameters
        CommonThreadParameters CTP(&gen, &files, p.path_str.c_str(), "Reports/GenericReport");

        std::vector<std::thread> threads;
        threads.push_back(std::thread(reportingFunction, &CTP));
        for(unsigned int i = 0; i < numThreads; i++){
            threads.push_back(std::thread(threadFunction, i, &CTP));
        }
        printf("All threads created\n");
        for(auto& thread : threads){
            //will block on the reporting thread before the worker threads
            thread.join();
        }

        printf("All threads joined, %u files (%llu bytes) processed\n", CTP.totalProcessed.load(), CTP.totalBytesValid.load());
    }
}