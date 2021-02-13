#include "PrunedAStarV2BruteForce.h"


//TODO - right now this only does the first two, but we could do more i guess
void PrunedAStarV2BruteForce::generateSuccessorStates(const Default_State_Type* state, const GCodeParser& gcp,
        const Default_LM_Type& lm, Brute_Force_PQ_Type& pq)
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
            continue;
        }else{
            //the index is a valid print transitions
            a1ValidAdjBitsets.push_back(a1AdjBitsetIndex);
        }
    }

    //since this check is needed in many items, we do it here instead
    for(Bitset_Index a2AdjBitsetIndex : lm.getAdjacentSegments(a2PosIndex)){
        if(state->getBitset().at(a2AdjBitsetIndex)){
            continue;
        }else{
            //the index is a valid print transitions
            a2ValidAdjBitsets.push_back(a2AdjBitsetIndex);
        }
    }

    //now check for dual print transitions
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
        
            if(isValidSegmentsPair(a1Segment, a2Segment)){
                //time to create the new segments and push it to the pq
                const Point3& a1NewPos = a1Segment.getOppositeEndpoint(a1Pos);
                const Point3& a2NewPos = a2Segment.getOppositeEndpoint(a2Pos);

                const Position_Index a1NewPosIndex = lm.getPosFromPoint3(a1NewPos);
                const Position_Index a2NewPosIndex = lm.getPosFromPoint3(a2NewPos);

                DynamicBitset newDBS = state->getBitset();
                newDBS.set(a1AdjBitsetIndex);
                newDBS.set(a2AdjBitsetIndex);
                RecomputeState newState(a1NewPosIndex, a2NewPosIndex, state->getDepth()+1, newDBS, state);
                pq.push(newState);
            }
        }
    }

    //now do the print no-op states
    //create the states where a1 prints and a2 NOOP
    for(Bitset_Index a1AdjBitsetIndex : a1ValidAdjBitsets){
        const GCP_Index a1GCPIndex = lm.getGCPFromBitset(a1AdjBitsetIndex);
        const GCodeSegment& a1Segment = gcp.at(a1GCPIndex);

        if(isValidSegmentNOOP(a1Segment, a2Pos)){
            //create and push the new state
            const Point3& a1NewPos = a1Segment.getOppositeEndpoint(a1Pos);
            const Position_Index a1NewPosIndex = lm.getPosFromPoint3(a1NewPos);

            DynamicBitset newDBS = state->getBitset();
            newDBS.set(a1AdjBitsetIndex);
            RecomputeState newState(a1NewPosIndex, a2PosIndex, state->getDepth()+1, newDBS, state);
            pq.push(newState);
        }
    }

    //create the states where a2 prints and a1 NOOP
    for(Bitset_Index a2AdjBitsetIndex : a2ValidAdjBitsets){
        const GCP_Index a2GCPIndex = lm.getGCPFromBitset(a2AdjBitsetIndex);
        const GCodeSegment& a2Segment = gcp.at(a2GCPIndex);

        if(isValidSegmentNOOP(a1Pos, a2Segment)){
            //create and push the new state
            const Point3& a2NewPos = a2Segment.getOppositeEndpoint(a2Pos);
            const Position_Index a2NewPosIndex = lm.getPosFromPoint3(a2NewPos);

            DynamicBitset newDBS = state->getBitset();
            newDBS.set(a2AdjBitsetIndex);
            RecomputeState newState(a1PosIndex, a2NewPosIndex, state->getDepth()+1, newDBS, state);
            pq.push(newState);
        }
    }
};