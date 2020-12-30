# include "RecomputeState.h"

RecomputeState::RecomputeState(unsigned int a1PosInd, unsigned int a2PosInd,
        unsigned int sd, const DynamicBitset& dbs) :
    agent1PositionIndex(a1PosInd), agent2PositionIndex(a2PosInd),
    stepDepth(sd), bitset(dbs)
{
}

unsigned int RecomputeState::distToGoalEst(void) const {
    return bitset.getUnsetCount() / 2;
}

unsigned int RecomputeState::totalPathLenEst(void) const {
    return stepDepth + distToGoalEst();
}

bool operator<(const RecomputeState& lhs, const RecomputeState& rhs){
    return &lhs < &rhs;
}