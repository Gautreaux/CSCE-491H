#include "PrunedAStar.h"

// #include <queue>
#include <vector>

#include "UtilLib/BiMap.h"
#include "UtilLib/PriorityQueue.h"

class StateCompare {
public:
    bool operator()(const RecomputeState& lhs, const RecomputeState& rhs){
        printf("DONG\n");
        return &lhs < &rhs;
    }
};

void prunedAStarLayer(const GCodeParser& gcp, const unsigned int layerStartInd, const unsigned int layerEndInd){
    //TODO
#ifdef DEBUG
    printf("Starting new layer z=%0.3f with %d segments\n", gcp.at(layerStartInd).getStartPoint().getZ(),
        layerEndInd - layerStartInd + 1);
#endif
    // indexes of the printed segments in the layer
    //  provides a implicit mapping of segment id to bitset position
    std::vector<unsigned int> printedSegmentsIndexes;  

    for(int i = layerStartInd; i <= layerEndInd; i++){
        auto seg = gcp.at(i);
        if(seg.isPrintSegment()){
            printedSegmentsIndexes.push_back(i);
        }
    }

#ifdef DEBUG
    printf("Layer resolved %d print segments\n", printedSegmentsIndexes.size());
#endif

    // print all the printSegments for this layer
    // for(auto ind : printedSegmentsIndexes){
    //     std::cout << ind << " " << gcp.at(ind) << std::endl;
    // }
    // std::cout << std::endl;

    //construct a position-int bi-map
    //  mostly so that we can get a set of all the positions
    BiMap<Point3, int> bimapPositionInt;
    unsigned int totalPositions = 0;
    for(auto segIndex : printedSegmentsIndexes){
        auto& sPos = gcp.at(segIndex).getStartPoint();
        auto& ePos = gcp.at(segIndex).getEndPoint();

        if(bimapPositionInt.countByA(sPos) == 0){
            bimapPositionInt.insert(sPos, totalPositions++);
        }

        if(bimapPositionInt.countByA(ePos) == 0){
            bimapPositionInt.insert(ePos, totalPositions++);
        }
    }

#ifdef DEBUG
    printf("Layer resolved %d total agent positions\n", totalPositions);
#endif

    // print all the positions in this layer
    // for(auto i = bimapPositionInt.findByABegin(); i != bimapPositionInt.findByAEnd(); i++){
    //     std::cout << i->first << std::endl;
    // }
    // std::cout << std::endl;

    //setup the pieces for A*
    // std::priority_queue<RecomputeState, std::vector<RecomputeState>, StateCompare> pq;
    // std::priority_queue<RecomputeState> pq;
    PriorityQueue<RecomputeState> pq;
    DynamicBitset startBitset(printedSegmentsIndexes.size());

    //generate all the starting points-pairings for this index
    //  TODO - for the moment, we add them all, but in the future we may want to sample a subset
    for(unsigned int i = 0; i < totalPositions; i++){
        for(unsigned int j = 0; j < totalPositions; j++){
            Point3& pi = bimapPositionInt.findByB(i)->second;
            Point3& pj = bimapPositionInt.findByB(j)->second;

            if(isValidPositionPair(pi, pj)){
                // std::cout << pi << " " << pj << std::endl;

                //pq.emplace(i, j, 0, startBitset);
                pq.push(RecomputeState(i, j, 0, startBitset));
            }
        }
    }

#ifdef DEBUG
    printf("Layer resolved %d total starting position pairs\n", pq.size());

    // //print all those pairs as reported by the priority queue
    // for(int i = 0; i < pq.size(); i++){
    //     const RecomputeState& state = pq.at(i);
    //     Point3& pi = bimapPositionInt.findByB(state.getA1PosIndex())->second;
    //     Point3& pj = bimapPositionInt.findByB(state.getA2PosIndex())->second;

    //     std::cout << pi << " " << pj << std::endl;
    // }
#endif

}

void prunedAStar(const GCodeParser& gcp){
    //TODO - any inital setup

    for(auto layer = gcp.layers_begin(); layer < gcp.layers_end(); layer++){
        // std::cout << "Layer: " << *layer << std::endl;
        //TODO - actually properly compute the start/end
        prunedAStarLayer(gcp, gcp.getLayerStartIndex(*layer), gcp.getLayerEndIndex(*layer));
    }
}