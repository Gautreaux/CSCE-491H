
#include <iostream>

#include "GCodeParser.h"
#include "PrunedAStar.h"

using std::cout;
using std::endl;

int main(int argc, char ** argv){
    cout << "HELLO THERE " << endl;

    GCodeParser gcp("cpp_impl/TestingFiles/simpleTest.gcode");
    //GCodeParser gcp("TestingFiles/simpleTest.gcode");

    if(gcp){
        printf("Found %d segments in the file.\n", gcp.numberSegments());
        if(gcp.numberSegments() < 30){
            for (auto it = gcp.segments_begin(); it != gcp.segments_end(); it++){
                cout << *it << endl;
            }
        }
    }

    prunedAStar(gcp);
}