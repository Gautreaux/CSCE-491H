
#include <iostream>

#include "GCodeParser.h"
#include "PrunedAStar.h"

using std::cout;
using std::endl;

int main(int argc, char ** argv){
    cout << "HELLO THERE " << endl;

    //GCodeParser gcp("cpp_impl/TestingFiles/simpleTest.gcode");
    //GCodeParser gcp("TestingFiles/simpleTest.gcode");
    
    //GCodeParser gcp("cpp_impl/TestingFiles/simpleRecomputable.gcode");
    GCodeParser gcp("TestingFiles/simpleRecomputable.gcode");

    if(gcp){
        printf("Found %d segments in %d layers in the file.\n", gcp.numberSegments(), gcp.numberZLayers());
        if(gcp.numberSegments() < 30){
            for (auto it = gcp.segments_begin(); it != gcp.segments_end(); it++){
                cout << *it << endl;
            }
            cout << endl;
        }
    }

    prunedAStar(gcp);
}