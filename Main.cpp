#include "pch.h"

#include <iostream>
#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>

#include "GCodeLib/GCodeParser.h"
#include "UtilLib/FileUtil.h"

#include "ChainStar.h"

#ifdef __NVCC__
#include "ChainLayerMetaAccelerator.cuh"
#endif

using std::cout;
using std::endl;

#define DEFAULT_FILEPATH "TestingFiles/simpleRecomputable.gcode"
#define NO_MODE_PROVIDED -1

#define DEFAULT_THREAD_COUNT 16

struct ParsedArgs{
    bool help;
    bool parseError;
    bool isDirectory;
    std::string path_str;
    std::string out_path_str;
    int mode;

    ParsedArgs() : help(false), parseError(false), 
        isDirectory(false), path_str(), out_path_str(),
        mode(NO_MODE_PROVIDED)
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
        try{
            int m = std::stoi(thisArg);
            if(p.mode == NO_MODE_PROVIDED){
                p.mode = m;
            }else{
                printf("Illegal mode detected after mode already set.\n");
                p.parseError = true;
            }
            continue;
        } catch (std::invalid_argument const&){
            //must be a path
            //do nothing
        }

        //now it must be a path
        if(p.path_str == ""){
            p.path_str = std::string(thisArg);
            if(p.path_str.back() != '/'){
                p.path_str.push_back('/');
            }
            continue;
        }else if(p.out_path_str == ""){
            p.out_path_str = std::string(thisArg);
            if(p.out_path_str.back() != '/'){
                p.out_path_str.push_back('/');
            }
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
    
    if(p.isDirectory && p.out_path_str == ""){
        printf("Must provide an out path.");
        p.parseError = true;
    }
    

    return p;
}

void printHelp(void){
    printf("TODO: help information\n");
    printf("-h : print help information\n");
    printf("-d : process the directory specified by <path>\n");
    printf("<path> : file/directory to process\n");
    printf("<mode> : integer in [0-3] for mode to run\n");
}

void printError(void){
    printf("An error occurred in argument processing, terminating\n");
}

struct CommonThreadParameters
{
public:
    std::mutex fileListLock;

    std::vector<std::string>& fileNames;

    std::atomic<unsigned int> totalFilesProcessed;
    std::atomic<unsigned long long> totalBytesValid;
    std::atomic<unsigned int> threadsRunning;
    std::atomic<bool> running;

    const std::string inBaseName;
    const std::string outBaseName;

    CommonThreadParameters(std::vector<std::string>& fileListRef,
        const std::string& inBase, const std::string& outBase
    ) :
    fileNames(fileListRef),
    totalFilesProcessed(0), totalBytesValid(0),
    threadsRunning(0), running(true),
    inBaseName(inBase), outBaseName(outBase)
    {};
};

void reportingFunction(const CommonThreadParameters *const CTP){
    printf("Reporting thread running\n");
}

void threadFunction(const unsigned int threadID, CommonThreadParameters *const CTP){
    printf("Thread: %d running\n", threadID);

    std::string s;

    while(true){
        CTP->fileListLock.lock();
        if(CTP->fileNames.size() == 0){
            CTP->fileListLock.unlock();
            CTP->threadsRunning.fetch_sub(1);
            if(CTP->threadsRunning.load() == 0){
                //technically could be run multiple times
                CTP->running.store(false);
            }
            return;
        }
        s = CTP->fileNames.back();
        CTP->fileNames.pop_back();
        CTP->fileListLock.unlock();


        const std::string fileID = s.substr(0, s.length() - strlen(".gcode"));
        printf("Thread %d procesing %s\n", threadID, fileID.c_str());

        // printf("%s\n", (CTP->inBaseName + s).c_str());
        GCodeParser gcp(CTP->inBaseName + s);

        if(gcp){
            for(unsigned int i = 0; i < 4; i++){
                if(i == 2){
                    continue;
                }
                const std::string outPath = CTP->outBaseName + fileID + "." + std::to_string(i) + ".out";


                printf("Thread %d completed mode %s:%d\n", threadID, fileID.c_str(), i);
            }
        }else{
            printf("Parse Error on %s\n", fileID.c_str());
        }

        CTP->totalFilesProcessed.fetch_add(1);
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
    printf("\tMode: %d\n", p.mode);

#ifdef __NVCC__
    logCUDAInfo();
#endif

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
            // if(gcp.numberSegments() < 30){
            //     for (auto it = gcp.segments_begin(); it != gcp.segments_end(); it++){
            //         cout << *it << endl;
            //     }
            //     cout << endl;
            // }
        }else{
            printf("gcp failed, exiting\n");
            exit(1);
        }

        // PrunedAStarV2SmartBrute p(gcp, 25, -1, 0.0);
        // p.doRecompute(gcp);
        // std::cout << "Total States Explored: " << p.getTotalStatesExpanded() << std::endl; 
        // std::cout << "Normal Exit occurred" << std::endl;

        // double z = *(gcp.layers_begin());
        // std::cout << "Testing on ChainLayerMeta on z = " << z << std::endl;
        // const ChainLayerMeta clm(gcp, z);

        // printf("Resolved %u print segments in layer.\n", clm.getNumPrintSegmentsInLayer());
        // printf("Resolved %lu chains in layer:\n", clm.getChainListRef().size());

        // if(clm.getChainListRef().size() < 30){
        //     int chainNum = 0;
        //     for(auto chain : clm.getChainListRef()){
        //         std::cout << "  " << chainNum++ << " " << chain << endl;
        //     }
        // }

        // // clm.doAllPairsCheck();
        // // clm.doAllChainsCheck();

        if(p.mode == 0){
            ChainStar<TheoreticalModel> csTheoretical;
            csTheoretical.doRecompute(gcp);
        }else if(p.mode == 1){
            ChainStar<CODEXModel> csCODEX;
            csCODEX.doRecompute(gcp);
        }else if(p.mode == 2){
            ChainStar<CurrentModel> csCurrent;
            csCurrent.doRecompute(gcp);
        }else if(p.mode == 3){
            ChainStar<RelaxedCurrentModel> csCurrent;
            csCurrent.doRecompute(gcp);
        }else{
            printf("FATAL: Illegal mode: %d\n", p.mode);
            throw std::runtime_error("Illegal mode");
        }

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

        // number of threads to spawn
        //  should become a command line parameter
        unsigned int numThreads = DEFAULT_THREAD_COUNT;
#ifdef FORCE_SINGLE_THREAD
        numThreads = 1;
#endif
        if(numThreads > 500){
            printf("NumThreads seems extremely high: %u\n", numThreads);
            exit(2);
        }

        CommonThreadParameters CTP(files, p.path_str, "");
        CTP.fileListLock.lock();

        std::vector<std::thread> threads;
        threads.push_back(std::thread(reportingFunction, &CTP));
        for(unsigned int i = 0; i < numThreads; i++){
            threads.push_back(std::thread(threadFunction, i, &CTP));
        }
        printf("All threads created\n");
        CTP.threadsRunning.store(numThreads);
        CTP.fileListLock.unlock();
        for(auto& thread : threads){
            //will block on the reporting thread before the worker threads
            thread.join();
        }

        printf("All threads joined, %u files (%llu bytes) processed\n", CTP.totalFilesProcessed.load(), CTP.totalBytesValid.load());
    }
}