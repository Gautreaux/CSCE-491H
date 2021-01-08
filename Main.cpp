
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

    //FYI - letting this run ~10million states with only dual moves.
    //  can get to ~330/652 printed on the first layer
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