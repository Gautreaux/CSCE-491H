#pragma once

#include "pch.h"

#include "GeometryLib/Point3.h"
#include "UtilLib/DynamicBitset.h"

class RecomputeState{
private:
    unsigned int agent1PositionIndex; // abbreviated index of agent 1 position
    unsigned int agent2PositionIndex; // abbreviated index of agent 2 position 
    unsigned int stepDepth; // number of successor states back to the goal
    DynamicBitset bitset;
public:
    RecomputeState(unsigned int a1PosInd, unsigned int a2PosInd, unsigned int sd, const DynamicBitset& dbs);

    // return estimated best-case distance to the goal
    unsigned int distToGoalEst(void) const;

    // return estimated best-case total path length
    unsigned int totalPathLenEst(void) const;

    unsigned int inline getA1PosIndex(void) const {return agent1PositionIndex;}
    unsigned int inline getA2PosIndex(void) const {return agent2PositionIndex;}
};

bool operator<(const RecomputeState& lhs, const RecomputeState& rhs);