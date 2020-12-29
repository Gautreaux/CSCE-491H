#pragma once

#include "pch.h"

#include "GeometryLib/Point3.h"
#include "UtilLib/DynamicBitset.h"

class RecomputeState{
private:
    Point3 agent1Position;
    Point3 agent2Position;
    int stepDepth; // number of successor states back to the goal
    DynamicBitset bitset;
public:
};