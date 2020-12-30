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
    RecomputeState(void);
    RecomputeState(unsigned int a1PosInd, unsigned int a2PosInd, unsigned int sd, const DynamicBitset& dbs);
    RecomputeState(const RecomputeState& other);
    RecomputeState(RecomputeState&& other);
    RecomputeState& operator=(const RecomputeState& other);
    RecomputeState& operator=(RecomputeState&& other);


    // return estimated best-case distance to the goal
    unsigned int distToGoalEst(void) const;

    // return estimated best-case total path length
    unsigned int totalPathLenEst(void) const;

    unsigned int inline getA1PosIndex(void) const {return agent1PositionIndex;}
    unsigned int inline getA2PosIndex(void) const {return agent2PositionIndex;}

    unsigned int inline getDepth(void) const {return stepDepth;}

    const DynamicBitset& getBitset(void) const {return bitset;}
};

bool operator<(const RecomputeState& lhs, const RecomputeState& rhs);