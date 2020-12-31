#include "PrunedAStar.h"


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
    PosIndexBiMap bimapPositionInt;
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
    
    // print all the positions in this layer
    // for(auto i = bimapPositionInt.findByABegin(); i != bimapPositionInt.findByAEnd(); i++){
    //     std::cout << i->first << std::endl;
    // }
    // std::cout << std::endl;
#endif

    //maps a positionIndex to concident segments index
    // i.e. position (1,1,1) <--> position_index 7 (the bimap above)
    //  printedSegmentsIndexes[0] --> segIndex[30] --> (1,1,1) to (2,2,2)
    //then printedSegmentsIndexes[7] = vector(..., 0, ...)
    //  store the index b/c this maps to the bitmap better
    PosSegMap positionAdjSegIndexMapping(totalPositions); 

    for(auto segIndex : printedSegmentsIndexes){
        const GCodeSegment& seg = gcp.at(segIndex);
        auto& sPos = seg.getStartPoint();
        auto& ePos = seg.getEndPoint();

        unsigned int sPosIndex = bimapPositionInt.findByA(sPos)->second;
        unsigned int ePosIndex = bimapPositionInt.findByA(ePos)->second;

        positionAdjSegIndexMapping[sPosIndex].push_back(segIndex);
        positionAdjSegIndexMapping[ePosIndex].push_back(segIndex);
    }

    //setup the pieces for A*
    // std::priority_queue<RecomputeState, std::vector<RecomputeState>, StateCompare> pq;
    // std::priority_queue<RecomputeState> pq;
    PriorityQueue<RecomputeState> pq;
    std::set<RecomputeState> visitedSet;
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

    unsigned int expandedStates = 0;
    bool foundGoal = false;
    unsigned int mostCompleteState = 0;

    //TODO - the major loop

    while(pq.size() > 0){
        const RecomputeState& state = pq.top();
        expandedStates += 1;

        //check if goal
        if(state.getBitset().getUnsetCount() == 0){
            const DynamicBitset& bs = state.getBitset();
            printf("GOAL BS: size %d, set count %d, unset count %d\n", bs.size(), bs.getSetCount(), bs.getUnsetCount());
            printf("Start BS: size %d, set count %d, unset count %d\n", startBitset.size(), startBitset.getSetCount(), startBitset.getUnsetCount());
            foundGoal = true;
            break;
        }

        mostCompleteState = std::max(mostCompleteState, state.getBitset().getSetCount());

        if(visitedSet.insert(state).second){
            //new element, time to expand
            updateSearchStates(state, pq, gcp, 
                    bimapPositionInt, positionAdjSegIndexMapping);
        }

        pq.pop();
        //printf("DBG: size %d\n", pq.size());
#ifdef DEBUG
        //reporting
        if(expandedStates % 50 == 0){
            printf("Total %d states expanded. ", expandedStates);
            printf("Pending states %d; Best state %d/%d printed.\n", pq.size(), mostCompleteState, printedSegmentsIndexes.size());
        }
#endif
    }

    if(foundGoal == false){
#ifdef DEBUG
        printf("Layer explored %d states and did not find a goal\n", expandedStates);
#endif
        //TODO - this should probably raise a runtime exception
        return;
    }

#ifdef DEBUG
    printf("Layer successfully found a goal after %d states at depth %d.\n", expandedStates, pq.top().getDepth());
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

void updateSearchStates(
    const RecomputeState& state, PriorityQueue<RecomputeState>& pq, GCodeParser gcp,
    PosIndexBiMap& bimapPositionIndex, PosSegMap& positionAdjSegIndexMapping)
{
    unsigned int a1PosIndex = state.getA1PosIndex(); //shorthand of the position of a1
    unsigned int a2PosIndex = state.getA2PosIndex(); //shorthand of the position of a2

    Point3 a1Pos = bimapPositionIndex.findByB(a1PosIndex)->second;
    Point3 a2Pos = bimapPositionIndex.findByB(a2PosIndex)->second;

    //find the state transitions where both agents can move to a new print
    for(auto a1AdjInd : positionAdjSegIndexMapping[a1PosIndex]){
        if(state.getBitset().at(a1AdjInd) == 1){
            continue;
        }
        for(auto a2AdjInd : positionAdjSegIndexMapping[a2PosIndex]){
            if(state.getBitset().at(a2AdjInd) == 1){
                continue;
            }
            if(isValidSegmentsPair(gcp.at(a1AdjInd), gcp.at(a2AdjInd))){
                //do the update

                //need to find the new ending points
                //TODO - this could be optimized with some pre work and another vector(s)
                Point3 a1EndPos = ((a1Pos == gcp.at(a1AdjInd).getStartPoint()) ?
                                    gcp.at(a1AdjInd).getEndPoint() :
                                    gcp.at(a1AdjInd).getEndPoint());
                Point3 a2EndPos = ((a2Pos == gcp.at(a2AdjInd).getStartPoint()) ?
                                    gcp.at(a2AdjInd).getEndPoint() :
                                    gcp.at(a2AdjInd).getEndPoint());

                unsigned int a1EndPosIndex = bimapPositionIndex.findByA(a1EndPos)->second;
                unsigned int a2EndPosIndex = bimapPositionIndex.findByA(a2EndPos)->second;

                //update the bitmap
                DynamicBitset dbs = state.getBitset();
                dbs.set(a1AdjInd, 1);
                dbs.set(a2AdjInd, 1);

                pq.push(RecomputeState(a1EndPosIndex, a2EndPosIndex, state.getDepth()+1, dbs));
            }
        }
    }

    //TODO - more transitions (in no real order)
    // a1 print, a2 noop
    // a1 print, a2 move
    // a2 print, a1 noop
    // a2 print, a1 move
    // a1 move, a2 noop
    // a1 move, a2 move
    // a2 move, a1 noop
    // a2 move, a1 move
    return;
}