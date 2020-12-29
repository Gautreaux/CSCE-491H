
#include <iostream>

#include "GCodeParser.h"

using std::cout;
using std::endl;

int main(int argc, char ** argv){
    cout << "HELLO THERE " << endl;

    GCodeParser gcp("cpp_impl/TestingFiles/simpleTest.gcode");

    if(gcp){
        printf("Found %d segments in the file.\n", gcp.numberSegments());
        if(gcp.numberSegments() < 30){
            for (auto it = gcp.begin(); it != gcp.end(); it++){
                cout << *it << endl;
            }
        }
    }

}