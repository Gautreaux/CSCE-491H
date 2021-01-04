#include "PrunedAStar.h"

#ifndef LOOP_PRINT_FREQUENCY
#ifdef DEBUG_1
#define LOOP_PRINT_FREQUENCY 1
#else
#define LOOP_PRINT_FREQUENCY 50
#endif
#endif

class StateCompare {
public:
    bool operator()(const RecomputeState& lhs, const RecomputeState& rhs){
        printf("DONG\n");
        return &lhs < &rhs;
    }
};



void prunedAStarLayer(const GCodeParser& gcp, double layer){

#ifdef DEBUG_3
    printf("Starting prunedAStarLayer\n");
#endif
    //build the various maps that allow us to index things efficiently
    LayerManager lm(gcp, layer);

    std::priority_queue<RecomputeState> pq;
    std::set<RecomputeState> visitedSet;
    unsigned int expandedStates = 0;
    bool foundGoal = false;
    unsigned int mostCompleteState = 0; // maximum number of printed segments in any explored state

    generateStartingPositions(gcp, lm, pq);

#ifdef DEBUG
    printf("Layer resolved %d total starting position pairs\n", pq.size());
// #ifdef DEBUG_4
//     //print all those pairs as reported by the priority queue
//     //No longer possible with transision to std::priority_queue
//     for(int i = 0; i < pq.size(); i++){
//         const RecomputeState& state = pq.at(i);
//         const Point3& pi = lm.getPoint3FromPos(state.getA1PosIndex());
//         const Point3& pj = lm.getPoint3FromPos(state.getA2PosIndex());
//         std::cout << pi << " " << pj << std::endl;
//     }
// #endif // DEBUG_4
#endif // DEBUG

    //TODO - should insert another start state(s) where
    //  one/both agents are starting without a print operation
    //  for the scenario where the entire object is printed by one agent

    // TODO (unrelated to above) - is there a potential deadlock where the
    //  agents are blocking eachother from finishing
    //  i.e. they are positioned at either endpoint of the final segment to print
    //      requiring one to move-no-print away so that the other can print


    while(pq.size() > 0){
        const RecomputeState state = pq.top();
        expandedStates += 1;

#ifdef DEBUG_4
        std::cout << "In Pruned A* Layer, got state " << state << std::endl;
#endif

        //check if goal
        if(state.getBitset().getUnsetCount() == 0){
#ifdef DEBUG
            const DynamicBitset& bs = state.getBitset();
            printf("GOAL BS: size %d, set count %d, unset count %d\n", bs.size(), bs.getSetCount(), bs.getUnsetCount());
            //printf("Start BS: size %d, set count %d, unset count %d\n", startBitset.size(), startBitset.getSetCount(), startBitset.getUnsetCount());
            printf("Depth: %d\n", state.getDepth());
#endif
            std::cout << std::endl; //force a flush
            foundGoal = true;
            break;
        }

        mostCompleteState = std::max(mostCompleteState, state.getBitset().getSetCount());

        if(visitedSet.insert(state).second){
#ifdef DEBUG_1
            std::cout << "Expanding state " << state << std::endl;
#endif
            //new element, time to expand
            updateSearchStates(state, gcp, lm, pq);

            // printf("State Expansion not yet implemeneted\n");
            // updateSearchStates(state, pq, gcp,
            //         bimapPositionInt, positionAdjSegIndexMapping,
            //         printedSegmentsIndexes);
        }

        // remove the item from the state
        pq.pop();

#ifdef DEBUG
        //reporting
        if(expandedStates % LOOP_PRINT_FREQUENCY == 0){
            printf("Total %d states expanded. ", expandedStates);
            printf("Pending states %d; Best state %d/%d printed.\n", pq.size(), mostCompleteState, lm.getTotalPrintSegments());
        }
        if(expandedStates > 100000){
            printf("100,000 states expanded without goal, terminating.\n");
            exit(99);
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
    //TODO - what type of return should this be?
}

void prunedAStar(const GCodeParser& gcp){
    //TODO - any inital setup

    for(auto layer = gcp.layers_begin(); layer < gcp.layers_end(); layer++){
        // std::cout << "Layer: " << *layer << std::endl;
        //TODO - actually properly compute the start/end
        //  What was that ^ one about?
        prunedAStarLayer(gcp, *layer);
    }
}

void updateSearchStates(
    const RecomputeState& state, const GCodeParser& gcp,
    const LayerManager& lm, std::priority_queue<RecomputeState>& pq)
{

#ifdef DEBUG_3
    std::cout << "Beginning updateSearchStates for " << state << std::endl;
    std::cout << "Beginning recompute state bitset: \n\t";
    state.getBitset().printBitData(std::cout);
    std::cout << std::endl;
    unsigned int newStatesAdded = 0;
#endif

    const Position_Index a1PosIndex = state.getA1PosIndex();
    const Position_Index a2PosIndex = state.getA2PosIndex();

    const Point3& a1Pos = lm.getPoint3FromPos(a1PosIndex);
    const Point3& a2Pos = lm.getPoint3FromPos(a2PosIndex);

#ifdef DEBUG_3
    std::cout << "Resolved positions: " << a1Pos << " " << a2Pos << std::endl;
#endif

    //first attempt to find states where both agents can move to a new position
    for(Bitset_Index a1AdjBitsetIndex : lm.getAdjacentSegments(a1PosIndex)){
        if(state.getBitset().at(a1AdjBitsetIndex) == 1){
#ifdef DEBUG_3
            std::cout << "Skipping seg (bitset_index) for A1 as it was already printed: " << a1AdjBitsetIndex << std::endl;
#endif
            continue;
        }
        const GCP_Index a1GCPIndex = lm.getGCPFromBitset(a1AdjBitsetIndex);
        const GCodeSegment& a1Segment = gcp.at(a1GCPIndex);

        for(Bitset_Index a2AdjBitsetIndex : lm.getAdjacentSegments(a2PosIndex)){
            if(state.getBitset().at(a2AdjBitsetIndex) == 1){
#ifdef DEBUG_3
                std::cout << "Skipping seg (bitset_index) for A2 as it was already printed: " << a2AdjBitsetIndex << std::endl;
#endif
                continue;
            }
            const GCP_Index a2GCPIndex = lm.getGCPFromBitset(a2AdjBitsetIndex);
            const GCodeSegment& a2Segment = gcp.at(a2GCPIndex);

            if(a1AdjBitsetIndex == a2AdjBitsetIndex){
                assert(a1GCPIndex == a2GCPIndex);
#ifdef DEBUG_3
                std::cout << "Skipping move pairs as they are moving the same segment: (bitset) " << a1AdjBitsetIndex << ", (gcp) " << a1GCPIndex << std::endl;
#endif
                continue;
            }

#ifdef DEBUG_3
            std::cout << "Found new dual-move segments pair (gcp_indexes): " << a1GCPIndex << " " << a2GCPIndex << std::endl;
#endif
            if(isValidSegmentsPair(a1Segment, a2Segment)){
                //time to create the new segments and push it to the pq
#ifdef DEBUG_3
                std::cout << "Dual move segments pair passed valid checks (gcp_indexes): " << a1GCPIndex << " " << a2GCPIndex << std::endl;
#endif

                // this could be made more efficient in some way
                const Point3& a1NewPos = a1Segment.getOppositeEndpoint(a1Pos);
                const Point3& a2NewPos = a2Segment.getOppositeEndpoint(a2Pos);

#ifdef DEBUG_3
                std::cout << "Post-move positions are: " << a1NewPos << " " << a2NewPos << std::endl;
#endif
#ifdef DEBUG
                if(a1NewPos == a2NewPos){
                    //This is actually a really big deal b/c it means that isValidSegmentsPair is incorrect
                    std::cout << "ERROR IN validSegments, new pos resolved to same position: " << a1NewPos << std::endl;
                    std::cout << "    Violating segments: a1: " << a1Segment << ", a2: " << a2Segment << std::endl;
                    std::cout << "    A1Pos: " << a1Pos << ", a2Pos: " << a2Pos << std::endl;
                    std::cout << "    a1NewPos: " << a1NewPos << ", a2NewPos: " << a2NewPos << std::endl;
                    continue;
                }
#endif

                const Position_Index a1NewPosIndex = lm.getPosFromPoint3(a1NewPos);
                const Position_Index a2NewPosIndex = lm.getPosFromPoint3(a2NewPos);

#ifdef DEBUG_3
                std::cout << "Post-move position indexes are: " << a1NewPosIndex << " " << a2NewPosIndex << std::endl;
#endif

                //update the bitset
                DynamicBitset newDBS = state.getBitset();
                newDBS.set(a1AdjBitsetIndex);
                newDBS.set(a2AdjBitsetIndex);
#ifdef DEBUG_1
                std::cout << "Pushing new state " << RecomputeState(a1NewPosIndex, a2NewPosIndex, state.getDepth()+1, newDBS) << std::endl;
#endif
                pq.push(RecomputeState(a1NewPosIndex, a2NewPosIndex, state.getDepth()+1, newDBS));
#ifdef DEBUG_3
                newStatesAdded += 1;
            }
            else {
                std::cout << "Dual-Move segments pair FAILED valid check (gcp_indexes): " << a1GCPIndex << " " << a2GCPIndex << std::endl;
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

#ifdef DEBUG_3
    std::cout << "Finishing state expansion, added # new states: " << newStatesAdded << std::endl;
#endif
    return;
}

void generateStartingPositions(const GCodeParser& gcp,
        const LayerManager& lm, std::priority_queue<RecomputeState>& pq)
{
    unsigned int totalPositions = lm.getTotalPositions();
    DynamicBitset startBitset(lm.getTotalPrintSegments());
#ifdef DEBUG_3
    std::cout << "Start Bitset is ";
    startBitset.printBitData(std::cout);
    std::cout << std::endl;
#endif

    //need all the starting pairs b/c what if agents are not interchangable
    for(unsigned int i = 0; i < totalPositions; i++){
        for(unsigned int j = 0; j < totalPositions; j++){
            if(i == j){
                continue;
            }

            const Point3& pi = lm.getPoint3FromPos(i);
            const Point3& pj = lm.getPoint3FromPos(j);

            if(isValidPositionPair(pi,pj)){
#ifdef DEBUG_3
                std::cout << "Adding new start point pair: " << pi << " " << pj << std::endl;
#endif
                pq.push(RecomputeState(i, j, 0, startBitset));
            }
        }
    }
}