#include "PrunedAStar.h"

void prunedAStarLayer(const GCodeParser& gcp, const unsigned int layerStartInd, const unsigned int layerEndInd){
    //TODO

    // indexes of the printed segments in the layer
    //  provides a implicit mapping of segment id to bitset position
    std::vector<unsigned int> printedSegmentsIndexes;  

    for(int i = layerStartInd; i <= layerEndInd; i++){
        auto seg = gcp.at(i);
        if(seg.isPrintSegment()){
            printedSegmentsIndexes.push_back(i);
        }
    }

    for(auto ind : printedSegmentsIndexes){
        std::cout << ind << " " << gcp.at(ind) << std::endl;
    }
}

void prunedAStar(const GCodeParser& gcp){
    //TODO - any inital setup

    for(auto layer = gcp.layers_begin(); layer < gcp.layers_end(); layer++){
        std::cout << "Layer: " << *layer << std::endl;
        //TODO - actually properly compute the start/end
        prunedAStarLayer(gcp, gcp.getLayerStartIndex(*layer), gcp.getLayerEndIndex(*layer));
    }
}