# include "LayerManager.h"

LayerManager::LayerManager(const GCodeParser& gcp, const double layer){
    GCP_Index layerStartIndex = gcp.getLayerStartIndex(layer);
    GCP_Index layerEndIndex = gcp.getLayerEndIndex(layer);
    this->layer = layer;

#ifdef DEBUG
    printf("Starting new layer z=%0.3f with %d segments\n", 
        layer, layerEndIndex - layerStartIndex + 1);
#ifdef DEBUG_4
    printf("\tStart,End index resolved to %d, %d",
        layerStartIndex, layerEndIndex);
#endif
#endif

    //extract only the print segments from the layer
    totalPrintSegments = 0;

    for(GCP_Index i = layerStartIndex; i <= layerEndIndex; i++){
        //TODO - debug checks that all segments are in the provided layer
        auto thisSeg = gcp.at(i);
        // std::cout << thisSeg << std::endl;
        if(thisSeg.isPrintSegment()){
            printedSegmentsTranslation.insert(i, totalPrintSegments);
            totalPrintSegments++;
        }
    }

#ifdef DEBUG
    printf("Layer resolved %d print segments\n", totalPrintSegments);
#ifdef DEBUG_4
    // print all the printSegments for this layer
    for(auto i = printedSegmentsTranslation.findByBBegin(); i != printedSegmentsTranslation.findByBEnd(); i++){
        std::cout << "GCP_Index: " << i->second << ", Bitset_Index: " << i->first << ", " << gcp.at(i->second) << std::endl;
    }
    std::cout << std::endl;
#endif
#endif

    if(totalPrintSegments == 0){
        return;
    }

    //resolve all the positions and build adjacency list
    totalPositions = 0;
    for(auto i = printedSegmentsTranslation.findByABegin(); i != printedSegmentsTranslation.findByAEnd(); i++){
        auto& startPosition = gcp.at(i->first).getStartPoint();
        auto& endPosition = gcp.at(i->first).getEndPoint();

        if(pointPositionIndexTranslation.countByA(startPosition) == 0){
            pointPositionIndexTranslation.insert(startPosition, totalPositions);
            assert(adjacentSegments.size() == totalPositions);
            adjacentSegments.push_back(Adjacent_Bitset_Indexes());
            adjacentSegments.back().push_back(i->second);
            totalPositions++;
        }else{
            auto t = pointPositionIndexTranslation.findByA(startPosition);
            adjacentSegments.at(t->second).push_back(i->second);
        }

        if(pointPositionIndexTranslation.countByA(endPosition) == 0){
            pointPositionIndexTranslation.insert(endPosition, totalPositions);
            assert(adjacentSegments.size() == totalPositions);
            adjacentSegments.push_back(Adjacent_Bitset_Indexes());
            adjacentSegments.back().push_back(i->second);
            totalPositions++;
        }else{
            auto t = pointPositionIndexTranslation.findByA(endPosition);
            adjacentSegments.at(t->second).push_back(i->second);
        }
    }

#ifdef DEBUG
    printf("Layer resolved %d total agent positions\n", totalPositions);

#ifdef DEBUG_4
    // print all the positions in this layer
    for(auto i = pointPositionIndexTranslation.findByBBegin(); i != pointPositionIndexTranslation.findByBEnd(); i++){
        std::cout << "Position Index: " << i->first << ", pos: " << i->second << std::endl;
    }
    std::cout << std::endl;

    // print the position adjacent
    std::cout << "Positions index adjacent Bitset Indexes mapping:" << std::endl;
    for(unsigned int i = 0; i < adjacentSegments.size(); i++){
        std::cout << "Position_Index: " << i << ", Pos: " << pointPositionIndexTranslation.findByB(i)->second << ", [";
        if(adjacentSegments.at(i).size() > 0){
            std::cout << adjacentSegments.at(i).at(0);
            for(unsigned int j = 1; j < adjacentSegments.at(i).size(); j++){
                std::cout << " " << adjacentSegments.at(i).at(j);
            }
        }
        std::cout << "]" << std::endl;

        for(unsigned int j = 0; j < adjacentSegments.at(i).size(); j++){
            auto t = adjacentSegments.at(i).at(j);
            std::cout << "\tBitset_Index: " << t;
            auto tt = printedSegmentsTranslation.findByB(t)->second;
            std::cout << ", GCP_Index: " << tt;
            std::cout << ", seg: " << gcp.at(tt);
            std::cout << std::endl;
        }
    }
    std::cout << std::endl;
#endif
#endif


}