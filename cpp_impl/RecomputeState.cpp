# include "RecomputeState.h"



unsigned int RecomputeState::distToGoalEst(void) const {
    return bitset.getUnsetCount() / 2;
}

unsigned int RecomputeState::totalPathLenEst(void) const {
    return stepDepth + distToGoalEst();
}