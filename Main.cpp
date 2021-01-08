
#include <iostream>

#include "GCodeParser.h"
#include "PrunedAStar.h"

using std::cout;
using std::endl;

#define DEFAULT_FILEPATH "TestingFiles/simpleRecomputable.gcode"

int main(int argc, char ** argv){
    cout << "HELLO THERE " << endl;

    std::string filePath;
    if(argc >= 2){
        filePath = std::string(argv[1]);
    }else{
        printf("No command line file path provided, using default.\n");
        filePath = DEFAULT_FILEPATH;
    }
    printf("Filepath resolved to %s\n", filePath.c_str());

    GCodeParser gcp(filePath);

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