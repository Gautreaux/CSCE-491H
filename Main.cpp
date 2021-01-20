
#include <filesystem>
#include <iostream>

#include "GCodeParser.h"
#include "PrunedAStar.h"

using std::cout;
using std::endl;

#define DEFAULT_FILEPATH "TestingFiles/simpleRecomputable.gcode"

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
    }else{
        //process a batch file
    }
}