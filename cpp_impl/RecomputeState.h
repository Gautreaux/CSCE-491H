#pragma once

#include "pch.h"

#include "GeometryLib/Point3.h"
#include "UtilLib/DynamicBitset.h"

class RecomputeState{
private:
    Point3 agent1Position;
    Point3 agent2Position;
    unsigned int stepDepth; // number of successor states back to the goal
    DynamicBitset bitset;
public:

    // return estimated best-case distance to the goal
    unsigned int distToGoalEst(void) const;

    // return estimated best-case total path length
    unsigned int totalPathLenEst(void) const;
};