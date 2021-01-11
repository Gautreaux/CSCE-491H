#include "PrunedAStar.h"

void prunedAStarLayer(const GCodeParser& gcp, double layer){

#ifdef DEBUG_3
    printf("Starting prunedAStarLayer\n");
#endif
    //build the various maps that allow us to index things efficiently
    LayerManager lm(gcp, layer);

    State_PQ pq;
    NonReallocVector<RecomputeState> visitedObjects;
    State_Set visitedSet;
    
    unsigned int expandedStates = 0; //TODO - really more of an explored states than expanded states
    const RecomputeState* foundGoal = nullptr;
    const RecomputeState* bestState = nullptr;
    unsigned int mostCompleteState = 0; // maximum number of printed segments in any explored state

    generateStartingPositions(gcp, lm, pq);

#ifdef DEBUG
    printf("Layer resolved %llu total starting position pairs\n", pq.size());
#endif // DEBUG

// #ifdef DEBUG
//     while(pq.size() > 0){
//         const RecomputeState state = pq.top();
//         std::cout << "Current state: " << state << std::endl;
//         pq.pop();
//     }
//     std::cout << std::endl;
//     return;
// #endif

    //TODO - should insert another start state(s) where
    //  one/both agents are starting without a print operation
    //  for the scenario where the entire object is printed by one agent

    // TODO (unrelated to above) - is there a potential deadlock where the
    //  agents are blocking eachother from finishing
    //  i.e. they are positioned at either endpoint of the final segment to print
    //      requiring one to move-no-print away so that the other can print


    StateGeneratorFunctionPtr stateGenerators[] = {
        updateSearchStatesDualPrints,
        updateSearchStatesSinglePrintNoOp,
        updateSearchStatesSinglePrintMove,
        updateSearchStatesRemainingStates
    };
    unsigned int currentGenerator = 0;
    const RecomputeState* lastBest = nullptr;

    while(currentGenerator < QTY_STATE_GENERATORS){
        if(pq.empty()){
            printf("Expanding %p at level %u\n", bestState, currentGenerator);
            stateGenerators[currentGenerator](bestState, gcp, lm, pq);
            printVerbose(std::cout, *bestState, gcp, lm);
        }
#ifdef DEBUG
        printf("Starting new general Expansion (lvl 0,1); best %p, states %llu\n", lastBest, pq.size());
#endif
        //now actually attempt expansion
        while(!pq.empty()){
            RecomputeState state = pq.top();

            // remove the item from the queue
            pq.pop();

            expandedStates += 1;

#ifdef DEBUG_1
            std::cout << "Current state: " << state << std::endl;
#endif

            //check if goal
            if(state.getBitset().getUnsetCount() == 0){
#ifdef DEBUG
                const DynamicBitset& bs = state.getBitset();
                printf("GOAL BS: size %d, set count %d, unset count %d\n", bs.size(), bs.getSetCount(), bs.getUnsetCount());
                //printf("Start BS: size %d, set count %d, unset count %d\n", startBitset.size(), startBitset.getSetCount(), startBitset.getUnsetCount());
                printf("Depth: %d\n", state.getDepth());
                std::cout << "Goal State: " << state << std::endl;
#endif
                std::cout << std::endl; //force a flush
                foundGoal = visitedObjects.push(state);
                break;
            }

            if((state.getDepth() + MAX_STEPBACK_ALLOWED) < mostCompleteState){
                //skip this state b/c it goes too far back
                // as a bonus, since most mostCompleteState is always increasing
                //  we do not need to add this to the visitedSet, saving memory
#ifdef DEBUG_3
                std::cout << "Skipping state (stepback): " << state << std::endl;
#endif
            }else{
                RecomputeState* statePtr = visitedObjects.push(state);
                if(state.getBitset().getSetCount() > mostCompleteState){
                    mostCompleteState = state.getBitset().getSetCount();
                    bestState = statePtr;
#ifdef DEBUG
                    printf("New best state: %u/%u, expanded: %d, pending: %llu\n",
                        state.getBitset().getSetCount(), lm.getTotalPrintSegments(),
                        expandedStates, pq.size());
#endif
                }

                if(visitedSet.insert(statePtr).second){
#ifdef DEBUG_1
                    std::cout << "Expanding state " << state << std::endl;
#endif
                    //new element, time to expand
                    stateGenerators[0](statePtr, gcp, lm, pq);
                    stateGenerators[1](statePtr, gcp, lm, pq);
                }else{
                    //this shouldn't be possible, right?
                    // the best state has to be new;
                    assert(statePtr != bestState);

                    // already in the state, so no need to store
                    visitedObjects.popLast();
                }
            }


#ifdef DEBUG
            //reporting
            if(expandedStates % LOOP_PRINT_FREQUENCY == 0){
                printf("Total %d states expanded. ", expandedStates);
                printf("Pending states %llu; Best state %u/%u printed.\n", pq.size(), mostCompleteState, lm.getTotalPrintSegments());
            }
            if(expandedStates > 100000000){
                printf("100,000,000 states expanded without goal, terminating.\n");
                exit(99);
            }
#endif
        }
        if(foundGoal == nullptr){
            if(lastBest == bestState){
                currentGenerator++;
            }else{
                currentGenerator = 2;
                lastBest = bestState;
            }
            // printf("Elevating state generation to %u, current best: %u/%u\n", currentGenerator, mostCompleteState, lm.getTotalPrintSegments());
        }
    }

    if(foundGoal == nullptr){
#ifdef DEBUG
        printf("Layer explored %d states and did not find a goal\n", expandedStates);
#endif
        //TODO - this should probably raise a runtime exception
        return;
    }

#ifdef DEBUG_1
    printf("At exit time, there were %llu elements left in the queue\n", pq.size());
    while(!pq.empty()){
        const RecomputeState state = pq.top();
        std::cout << "Current state: " << state << std::endl;

        //std::cout << "PriorityQueue size (pre-pop) : " << pq.size() << std::endl;
        pq.pop();
        //std::cout << "PriorityQueue size (post-pop): " << pq.size() << std::endl;
    }
    std::cout << std::endl;
#endif

#ifdef DEBUG
    printf("Layer successfully found a goal after %d states at depth %d.\n", expandedStates, pq.top().getDepth());
#endif

    // now we need to extract the path from the states
    std::vector<const RecomputeState*> resolvedPath;
    while(foundGoal != nullptr){
        resolvedPath.push_back(foundGoal);
        foundGoal = foundGoal->getParent();
    }

    std::cout << "Resolved path:" << std::endl;
    for(auto i = resolvedPath.rbegin(); i != resolvedPath.rend(); i++){
        printVerbose(std::cout, **i, gcp, lm);
        // std::cout << **i << std::endl;
    }

    //TODO - what type of return should this be?
}

void prunedAStar(const GCodeParser& gcp){
    //TODO - any inital setup

    for(auto layer = gcp.layers_begin(); layer < gcp.layers_end(); layer++){
        // std::cout << "Layer: " << *layer << std::endl;
        //TODO - actually properly compute the start/end
        //  What was that ^ one about?
        prunedAStarLayer(gcp, *layer);
        break;
    }
}

void generateStartingPositions(const GCodeParser& gcp,
        const LayerManager& lm, State_PQ& pq)
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
                RecomputeState newState(i, j, 0, startBitset, nullptr);
#ifdef DEBUG_1
                std::cout << "Pushing new state " << newState << std::endl;
#endif
                //std::cout << "PriorityQueue size (pre-push) : " << pq.size() << std::endl;
                pq.push(newState);
                //std::cout << "PriorityQueue size (post-push): " << pq.size() << std::endl;
            }
        }
    }
}

void printVerbose(std::ostream& os, const RecomputeState& state, const GCodeParser& gcp, const LayerManager& lm){
    os << "A1: " << lm.getPoint3FromPos(state.getA1PosIndex()) << "(" << state.getA1PosIndex() << "), ";
    os << "A2: " << lm.getPoint3FromPos(state.getA2PosIndex()) << "(" << state.getA2PosIndex() << "), ";
    os << "Depth: " << state.getDepth() << ", ";
    os << "BitData: ";
    state.getBitset().printBitData(os);
    os << std::endl;
}

//add all the dual print moves to the queue
void updateSearchStatesDualPrints(
    const RecomputeState* state, const GCodeParser& gcp,
    const LayerManager& lm, State_PQ& pq)
{
    const Position_Index a1PosIndex = state->getA1PosIndex();
    const Position_Index a2PosIndex = state->getA2PosIndex();

    const Point3& a1Pos = lm.getPoint3FromPos(a1PosIndex);
    const Point3& a2Pos = lm.getPoint3FromPos(a2PosIndex);


     //pre-caching which of the adjacent segments are valid transitions
    std::vector<Bitset_Index> a1ValidAdjBitsets;
    std::vector<Bitset_Index> a2ValidAdjBitsets;

    //since this check is needed in many items, we do it here instead
    for(Bitset_Index a1AdjBitsetIndex : lm.getAdjacentSegments(a1PosIndex)){
        if(state->getBitset().at(a1AdjBitsetIndex)){
#ifdef DEBUG_3
            std::cout << "Skipping seg (bitset_index) for A1 as it was already printed: " << a1AdjBitsetIndex << std::endl;
#endif
            continue;
        }else{
            //the index is a valid print transitions
            a1ValidAdjBitsets.push_back(a1AdjBitsetIndex);
        }
    }

    //since this check is needed in many items, we do it here instead
    for(Bitset_Index a2AdjBitsetIndex : lm.getAdjacentSegments(a2PosIndex)){
        if(state->getBitset().at(a2AdjBitsetIndex)){
#ifdef DEBUG_3
            std::cout << "Skipping seg (bitset_index) for A2 as it was already printed: " << a2AdjBitsetIndex << std::endl;
#endif
            continue;
        }else{
            //the index is a valid print transitions
            a2ValidAdjBitsets.push_back(a2AdjBitsetIndex);
        }
    }

    for(Bitset_Index a1AdjBitsetIndex : a1ValidAdjBitsets){
        const GCP_Index a1GCPIndex = lm.getGCPFromBitset(a1AdjBitsetIndex);
        const GCodeSegment& a1Segment = gcp.at(a1GCPIndex);

        for(Bitset_Index a2AdjBitsetIndex : a2ValidAdjBitsets){
            const GCP_Index a2GCPIndex = lm.getGCPFromBitset(a2AdjBitsetIndex);
            const GCodeSegment& a2Segment = gcp.at(a2GCPIndex);

            if(a1AdjBitsetIndex == a2AdjBitsetIndex){
                assert(a1GCPIndex == a2GCPIndex);
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
                DynamicBitset newDBS = state->getBitset();
                newDBS.set(a1AdjBitsetIndex);
                newDBS.set(a2AdjBitsetIndex);
                RecomputeState newState(a1NewPosIndex, a2NewPosIndex, state->getDepth()+1, newDBS, state);
#ifdef DEBUG_1
                std::cout << "Pushing new state " << newState << std::endl;
#endif
                pq.push(newState);
            }
        }
    }
}

//add all the print noop states to the queue
void updateSearchStatesSinglePrintNoOp(
    const RecomputeState* state, const GCodeParser& gcp,
    const LayerManager& lm, State_PQ& pq)
{
    const Position_Index a1PosIndex = state->getA1PosIndex();
    const Position_Index a2PosIndex = state->getA2PosIndex();

    const Point3& a1Pos = lm.getPoint3FromPos(a1PosIndex);
    const Point3& a2Pos = lm.getPoint3FromPos(a2PosIndex);

    
    //pre-caching which of the adjacent segments are valid transitions
    std::vector<Bitset_Index> a1ValidAdjBitsets;
    std::vector<Bitset_Index> a2ValidAdjBitsets;

    //since this check is needed in many items, we do it here instead
    for(Bitset_Index a1AdjBitsetIndex : lm.getAdjacentSegments(a1PosIndex)){
        if(state->getBitset().at(a1AdjBitsetIndex)){
#ifdef DEBUG_3
            std::cout << "Skipping seg (bitset_index) for A1 as it was already printed: " << a1AdjBitsetIndex << std::endl;
#endif
            continue;
        }else{
            //the index is a valid print transitions
            a1ValidAdjBitsets.push_back(a1AdjBitsetIndex);
        }
    }

    //since this check is needed in many items, we do it here instead
    for(Bitset_Index a2AdjBitsetIndex : lm.getAdjacentSegments(a2PosIndex)){
        if(state->getBitset().at(a2AdjBitsetIndex)){
#ifdef DEBUG_3
            std::cout << "Skipping seg (bitset_index) for A2 as it was already printed: " << a2AdjBitsetIndex << std::endl;
#endif
            continue;
        }else{
            //the index is a valid print transitions
            a2ValidAdjBitsets.push_back(a2AdjBitsetIndex);
        }
    }

    //create the states where a1 prints and a2 NOOP
    for(Bitset_Index a1AdjBitsetIndex : a1ValidAdjBitsets){
        const GCP_Index a1GCPIndex = lm.getGCPFromBitset(a1AdjBitsetIndex);
        const GCodeSegment& a1Segment = gcp.at(a1GCPIndex);

        if(isValidSegNOOP(a1Segment, a2Pos)){
            //create and push the new state
            const Point3& a1NewPos = a1Segment.getOppositeEndpoint(a1Pos);
            const Position_Index a1NewPosIndex = lm.getPosFromPoint3(a1NewPos);

            DynamicBitset newDBS = state->getBitset();
            newDBS.set(a1AdjBitsetIndex);
            RecomputeState newState(a1NewPosIndex, a2PosIndex, state->getDepth()+1, newDBS, state);
#ifdef DEBUG_1
            std::cout << "Pushing new state " << newState << std::endl;
#endif
            pq.push(newState);
        }
    }

    //create the states where a2Prints and a1 NOOP
    for(Bitset_Index a2AdjBitsetIndex : a2ValidAdjBitsets){
        const GCP_Index a2GCPIndex = lm.getGCPFromBitset(a2AdjBitsetIndex);
        const GCodeSegment& a2Segment = gcp.at(a2GCPIndex);

        if(isValidSegNOOP(a1Pos, a2Segment)){
            //create and push the new state
            const Point3& a2NewPos = a2Segment.getOppositeEndpoint(a2Pos);
            const Position_Index a2NewPosIndex = lm.getPosFromPoint3(a2NewPos);

            DynamicBitset newDBS = state->getBitset();
            newDBS.set(a2AdjBitsetIndex);
            RecomputeState newState(a1PosIndex, a2NewPosIndex, state->getDepth()+1, newDBS, state);
#ifdef DEBUG_1
            std::cout << "Pushing new state " << newState << std::endl;
#endif
            pq.push(newState);
        }
    }
}

//add all the print single move to the queue
void updateSearchStatesSinglePrintMove(
    const RecomputeState* state, const GCodeParser& gcp,
    const LayerManager& lm, State_PQ& pq)
{
    const Position_Index a1PosIndex = state->getA1PosIndex();
    const Position_Index a2PosIndex = state->getA2PosIndex();

    const Point3& a1Pos = lm.getPoint3FromPos(a1PosIndex);
    const Point3& a2Pos = lm.getPoint3FromPos(a2PosIndex);

#ifdef DEBUG_3
    std::cout << "Resolved positions: " << a1Pos << " " << a2Pos << std::endl;
#endif

    //pre-caching which of the adjacent segments are valid transitions
    std::vector<Bitset_Index> a1ValidAdjBitsets;
    std::vector<Bitset_Index> a2ValidAdjBitsets;

    //since this check is needed in many items, we do it here instead
    for(Bitset_Index a1AdjBitsetIndex : lm.getAdjacentSegments(a1PosIndex)){
        if(state->getBitset().at(a1AdjBitsetIndex)){
#ifdef DEBUG_3
            std::cout << "Skipping seg (bitset_index) for A1 as it was already printed: " << a1AdjBitsetIndex << std::endl;
#endif
            continue;
        }else{
            //the index is a valid print transitions
            a1ValidAdjBitsets.push_back(a1AdjBitsetIndex);
        }
    }

    //since this check is needed in many items, we do it here instead
    for(Bitset_Index a2AdjBitsetIndex : lm.getAdjacentSegments(a2PosIndex)){
        if(state->getBitset().at(a2AdjBitsetIndex)){
#ifdef DEBUG_3
            std::cout << "Skipping seg (bitset_index) for A2 as it was already printed: " << a2AdjBitsetIndex << std::endl;
#endif
            continue;
        }else{
            //the index is a valid print transitions
            a2ValidAdjBitsets.push_back(a2AdjBitsetIndex);
        }
    }

    // create the list of new positions that are incident on
    //  an unprinted segment
    std::vector<Position_Index> posIndexUnprintedIncident;
    for(auto newPosIter = lm.getPointsStartIterator(); newPosIter != lm.getPointsEndIterator(); newPosIter++){
        // const Point3& newPos = newPosIter->first;
        const Position_Index newPosIndex = newPosIter->second;

        for(Bitset_Index possibleAdjBitsetIndex : lm.getAdjacentSegments(newPosIndex)){
            if(state->getBitset().at(possibleAdjBitsetIndex)){
                //it is set, do nothing
            }else{
                //for this newPos there is a new segment that 
                //  that is incident and not printed
                posIndexUnprintedIncident.push_back(newPosIndex);
                break; // we only care if there is one, not what it is
            }
        }
    }

    //create the states where a1 prints and a2 move to a new position
    for(Bitset_Index a1AdjBitsetIndex : a1ValidAdjBitsets){
        //TODO - these two lookups are repeated several times, perhaps they should be cached
        const GCP_Index a1GCPIndex = lm.getGCPFromBitset(a1AdjBitsetIndex);
        const GCodeSegment& a1Segment = gcp.at(a1GCPIndex);

        const Point3& a1NewPos = a1Segment.getOppositeEndpoint(a1Pos);
        const Position_Index a1NewPosIndex = lm.getPosFromPoint3(a1NewPos);

        //now we want to find the set of all possible new a2 positions where
        //  1. the new position is incident on an unprinted segment (above)
        //  2. the transition from current pos to new pos does not intersect the 
        //       segment to be printed by a1
        for(Position_Index a2NewPosIndex : posIndexUnprintedIncident){
            GCodeSegment newSegment(a2Pos, lm.getPoint3FromPos(a2NewPosIndex), 0);
            if(isValidSegmentsPair(a1Segment, newSegment)){
                DynamicBitset newDBS = state->getBitset();
                newDBS.set(a1AdjBitsetIndex);
                RecomputeState newState(a1NewPosIndex, a2NewPosIndex, state->getDepth()+1, newDBS, state);
#ifdef DEBUG_1
                std::cout << "Pushing new state " << newState << std::endl;
#endif
                pq.push(newState);
            }
        }

    }

    //create the states where a2 prints and a1 move to a new position
    for(Bitset_Index a2AdjBitsetIndex : a2ValidAdjBitsets){
        //TODO - these two lookups are repeated several times, perhaps they should be cached
        const GCP_Index a2GCPIndex = lm.getGCPFromBitset(a2AdjBitsetIndex);
        const GCodeSegment& a2Segment = gcp.at(a2GCPIndex);

        const Point3& a2NewPos = a2Segment.getOppositeEndpoint(a2Pos);
        const Position_Index a2NewPosIndex = lm.getPosFromPoint3(a2NewPos);

        //now we want to find the set of all possible new a1 positions where
        //  1. the new position is incident on an unprinted segment (above)
        //  2. the transition from current pos to new pos does not intersect the 
        //       segment to be printed by a2
        for(Position_Index a1NewPosIndex : posIndexUnprintedIncident){
            GCodeSegment newSegment(a1Pos, lm.getPoint3FromPos(a1NewPosIndex), 0);
            if(isValidSegmentsPair(newSegment, a2Segment)){
                DynamicBitset newDBS = state->getBitset();
                newDBS.set(a2AdjBitsetIndex);
                RecomputeState newState(a1NewPosIndex, a2NewPosIndex, state->getDepth()+1, newDBS, state);
#ifdef DEBUG_1
                std::cout << "Pushing new state " << newState << std::endl;
#endif
                pq.push(newState);
            }
        }
    }

}

//desperatelly add any possible state to the queue
//  hopefully never called
void updateSearchStatesRemainingStates(
    const RecomputeState* state, const GCodeParser& gcp,
    const LayerManager& lm, State_PQ& pq)
{
    const Position_Index a1PosIndex = state->getA1PosIndex();
    const Position_Index a2PosIndex = state->getA2PosIndex();

    const Point3& a1Pos = lm.getPoint3FromPos(a1PosIndex);
    const Point3& a2Pos = lm.getPoint3FromPos(a2PosIndex);

    // create the list of new positions that are incident on
    //  an unprinted segment
    std::vector<Position_Index> posIndexUnprintedIncident;
    for(auto newPosIter = lm.getPointsStartIterator(); newPosIter != lm.getPointsEndIterator(); newPosIter++){
        // const Point3& newPos = newPosIter->first;
        const Position_Index newPosIndex = newPosIter->second;

        for(Bitset_Index possibleAdjBitsetIndex : lm.getAdjacentSegments(newPosIndex)){
            if(state->getBitset().at(possibleAdjBitsetIndex)){
                //it is set, do nothing
            }else{
                //for this newPos there is a new segment that 
                //  that is incident and not printed
                posIndexUnprintedIncident.push_back(newPosIndex);
                break; // we only care if there is one, not what it is
            }
        }
    }

    // now for the states that are going to really balloon things
    // these "work" but its a absurd amount of states
    //  something like 3*(n^2) where n = number of segments in the layer
    //  i.e. if layer has 577 segments, 1 million states are generated
    //      (some reduction happens as segments are printed) 
    // TODO - develop some pruning method for these

    // a1 move & a2 move to new spots incident on unprinted segments
    for(Position_Index a1NewPosIndex : posIndexUnprintedIncident){
        const Point3& a1NewPos = lm.getPoint3FromPos(a1NewPosIndex);
        GCodeSegment newA1Seg(a1Pos, a1NewPos, 0);

        for(Position_Index a2NewPosIndex : posIndexUnprintedIncident){
            if(a1NewPosIndex == a2NewPosIndex){
                continue;
            }
            const Point3& a2NewPos = lm.getPoint3FromPos(a2NewPosIndex);
            GCodeSegment newA2Seg(a2Pos, a2NewPos, 0);

            if(isValidSegmentsPair(newA1Seg, newA2Seg)){
                RecomputeState newState(a1NewPosIndex, a2NewPosIndex, state->getDepth()+1, state->getBitset(), state);
#ifdef DEBUG_1
                std::cout << "Pushing new state " << newState << std::endl;
#endif
                pq.push(newState);
            }
        }
    }

    //a1 move, a2 noop
    for(Position_Index a1NewPosIndex : posIndexUnprintedIncident){
        const Point3& a1NewPos = lm.getPoint3FromPos(a1NewPosIndex);
        GCodeSegment newA1Seg(a1Pos, a1NewPos, 0);

        if(isValidSegNOOP(newA1Seg, a2Pos)){
            RecomputeState newState(a1NewPosIndex, a2PosIndex, state->getDepth()+1, state->getBitset(), state);
#ifdef DEBUG_1
            std::cout << "Pushing new state " << newState << std::endl;
#endif
            pq.push(newState);
        }
    }

    // a2 move, a1 noop
    for(Position_Index a2NewPosIndex : posIndexUnprintedIncident){
        const Point3& a2NewPos = lm.getPoint3FromPos(a2NewPosIndex);
        GCodeSegment newA2Seg(a2Pos, a2NewPos, 0);
        if(isValidSegNOOP(a1Pos, newA2Seg)){
            RecomputeState newState(a1PosIndex, a2NewPosIndex, state->getDepth()+1, state->getBitset(), state);
#ifdef DEBUG_1
            std::cout << "Pushing new state " << newState << std::endl;
#endif
            pq.push(newState);
        }
    }
}