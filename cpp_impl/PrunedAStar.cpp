#include "PrunedAStar.h"

#ifdef DEBUG_3
#define LOOP_PRINT_FREQUENCY 1
#else
#define LOOP_PRINT_FREQUENCY 50
#endif


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

#ifdef DEBUG_4
    // print all the positions in this layer
    for(auto i = bimapPositionInt.findByABegin(); i != bimapPositionInt.findByAEnd(); i++){
        std::cout << i->first << std::endl;
    }
    std::cout << std::endl;
#endif
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
    //  some form of vertex cover where we take n positions s.t.
    //      each segment is covered a maximal number of times
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

    //TODO - should insert another start state(s) where 
    //  one/both agents are starting without a print operation 
    //  for the scenario where the entire object is printed by one agent

    // TODO (unrelated to above) - is there a potential deadlock where the 
    //  agents are blocking eachother from finishing   
    //  i.e. they are positioned at either endpoint of the final segment to print
    //      requiring one to move-no-print away so that the other can print
#ifdef DEBUG
    printf("Layer resolved %d total starting position pairs\n", pq.size());

#ifdef DEBUG_4
    //print all those pairs as reported by the priority queue
    for(int i = 0; i < pq.size(); i++){
        const RecomputeState& state = pq.at(i);
        Point3& pi = bimapPositionInt.findByB(state.getA1PosIndex())->second;
        Point3& pj = bimapPositionInt.findByB(state.getA2PosIndex())->second;

        std::cout << pi << " " << pj << std::endl;
    }
#endif
#endif

    unsigned int expandedStates = 0;
    bool foundGoal = false;
    unsigned int mostCompleteState = 0; // maximum number of printed segments in any explored state

    //TODO - the major loop

    while(pq.size() > 0){
        const RecomputeState& state = pq.top();
        expandedStates += 1;

#ifdef DEBUG_4
        std::cout << "In Pruned A* Layer, got state " << state << std::endl;
#endif

        //check if goal
        if(state.getBitset().getUnsetCount() == 0){
#ifdef DEBUG
            const DynamicBitset& bs = state.getBitset();
            printf("GOAL BS: size %d, set count %d, unset count %d\n", bs.size(), bs.getSetCount(), bs.getUnsetCount());
            printf("Start BS: size %d, set count %d, unset count %d\n", startBitset.size(), startBitset.getSetCount(), startBitset.getUnsetCount());
            printf("Depth: %d\n", state.getDepth());
#endif
            std::cout << std::endl; //force a flush
            foundGoal = true;
            break;
        }

        mostCompleteState = std::max(mostCompleteState, state.getBitset().getSetCount());

        if(visitedSet.insert(state).second){
#ifdef DEBUG_4
            std::cout << "Expanding state " << state << std::endl;
#endif
            //new element, time to expand
            updateSearchStates(state, pq, gcp, 
                    bimapPositionInt, positionAdjSegIndexMapping, 
                    printedSegmentsIndexes);
        }

        pq.pop();
        //printf("DBG: size %d\n", pq.size());
#ifdef DEBUG
        //reporting
        if(expandedStates % LOOP_PRINT_FREQUENCY == 0){
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
    PosIndexBiMap& bimapPositionIndex, PosSegMap& positionAdjSegIndexMapping,
    const std::vector<unsigned int>& printedSegmentsIndexes)
{

#ifdef DEBUG_4
    std::cout << "Beginning updateSearchStates for " << state << std::endl;
    std::cout << "Beginning recompute state bitset: \n\t";
    state.getBitset().printBitData(std::cout);
    std::cout << std::endl;
    unsigned int newStatesAdded = 0;
#endif

    unsigned int a1PosIndex = state.getA1PosIndex(); //shorthand of the position of a1
    unsigned int a2PosIndex = state.getA2PosIndex(); //shorthand of the position of a2

    Point3 a1Pos = bimapPositionIndex.findByB(a1PosIndex)->second;
    Point3 a2Pos = bimapPositionIndex.findByB(a2PosIndex)->second;

#ifdef DEBUG_4
    std::cout << "Resolved positions: " << a1Pos << " " << a2Pos << std::endl;
    std::cout << "A1 adj segments:";
    for(auto a1AdjInd : positionAdjSegIndexMapping[a1PosIndex]){
        std::cout << " " << a1AdjInd;
    }
    std::cout << std::endl;

    std::cout << "A2 adj segments:";
    for(auto a2AdjInd : positionAdjSegIndexMapping[a2PosIndex]){
        std::cout << " " << a2AdjInd;
    }
    std::cout << std::endl;
#endif

    //find the state transitions where both agents can move to a new print
    //TODO - the get bitest is wrong
    for(auto a1AdjInd : positionAdjSegIndexMapping[a1PosIndex]){

        //TODO - this is temporary, but we need to translate a segment index into the bitset index
        unsigned int a1BSIndex;
        for(unsigned int i=0; i < printedSegmentsIndexes.size(); i++){
            if(printedSegmentsIndexes[i] == a1AdjInd){
                a1BSIndex = i;
                break;
            }
        }

#ifdef DEBUG_4
            std::cout << "a1AdjInd translated to a1BSIndex : " << a1AdjInd << " -> " << a1BSIndex << std::endl;
#endif

        if(state.getBitset().at(a1BSIndex) == 1){
#ifdef DEBUG_4
            std::cout << "Skipping seg (index) for A1 as it was already printed: " << a1AdjInd << std::endl; 
#endif
            continue;
        }
        for(auto a2AdjInd : positionAdjSegIndexMapping[a2PosIndex]){
            //TODO - this is temporary, but we need to translate a segment index into the bitset index
            unsigned int a2BSIndex;
            for(unsigned int i=0; i < printedSegmentsIndexes.size(); i++){
                if(printedSegmentsIndexes[i] == a2AdjInd){
                    a2BSIndex = i;
                    break;
                }
            }
#ifdef DEBUG_4
            std::cout << "a2AdjInd translated to a2BSIndex : " << a2AdjInd << " -> " << a2BSIndex << std::endl;
#endif

            if(state.getBitset().at(a2BSIndex) == 1){
#ifdef DEBUG_4
                std::cout << "Skipping seg (index) for A2 as it was already printed: " << a2AdjInd << std::endl; 
#endif
                continue;
            }

            if(a1AdjInd == a2AdjInd){
            #ifdef DEBUG_4
                std::cout << "Skipping move pairs as they are moving same segment: " << a1AdjInd << std::endl;
            #endif
                continue;
            }

#ifdef DEBUG_4
            std::cout << "Found new dual-move segments pair (indexes): " << a1AdjInd << " " << a2AdjInd << std::endl;
#endif

            if(isValidSegmentsPair(gcp.at(a1AdjInd), gcp.at(a2AdjInd))){
                //do the update

#ifdef DEBUG_4
                std::cout << "Dual-Move segments pair passed valid check (indexes): " << a1AdjInd << " " << a2AdjInd << std::endl;
#endif
                //need to find the new ending points
                //TODO - this could be optimized with some pre work and another vector(s)
                Point3 a1EndPos = ((a1Pos == gcp.at(a1AdjInd).getStartPoint()) ?
                                    gcp.at(a1AdjInd).getEndPoint() :
                                    gcp.at(a1AdjInd).getEndPoint());
                Point3 a2EndPos = ((a2Pos == gcp.at(a2AdjInd).getStartPoint()) ?
                                    gcp.at(a2AdjInd).getEndPoint() :
                                    gcp.at(a2AdjInd).getEndPoint());

                //TODO - not sure if this is correct indexing either
                unsigned int a1EndPosIndex = bimapPositionIndex.findByA(a1EndPos)->second;
                unsigned int a2EndPosIndex = bimapPositionIndex.findByA(a2EndPos)->second;

                //update the bitset
                DynamicBitset dbs = state.getBitset();
                dbs.set(a1AdjInd, 1);
                dbs.set(a2AdjInd, 1);

                pq.push(RecomputeState(a1EndPosIndex, a2EndPosIndex, state.getDepth()+1, dbs));
#ifdef DEBUG_4
                newStatesAdded += 1;
            }
            else {
                std::cout << "Dual-Move segments pair FAILED valid check (indexes): " << a1AdjInd << " " << a2AdjInd << std::endl;
#endif
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

#ifdef DEBUG_4
    std::cout << "Finishing state expansion, added # new states: " << newStatesAdded << std::endl;
#endif
    return;
}