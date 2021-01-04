#pragma once

#include "pch.h"

#include "GeometryLib/Point3.h"
#include "LayerManager.h"
#include "UtilLib/DynamicBitset.h"

class RecomputeState{
private:
    Position_Index agent1PositionIndex; // abbreviated index of agent 1 position
    Position_Index agent2PositionIndex; // abbreviated index of agent 2 position 
    unsigned int stepDepth; // number of successor states back to the goal
    DynamicBitset bitset;
    const RecomputeState* parentState;
public:
    RecomputeState(void);
    RecomputeState(Position_Index a1PosInd, Position_Index a2PosInd, 
        unsigned int sd, const DynamicBitset& dbs, const RecomputeState* parent);
    RecomputeState(const RecomputeState& other);
    RecomputeState(RecomputeState&& other);
    RecomputeState& operator=(const RecomputeState& other);
    RecomputeState& operator=(RecomputeState&& other);


    // return estimated best-case distance to the goal
    unsigned int distToGoalEst(void) const;

    // return estimated best-case total path length
    unsigned int totalPathLenEst(void) const;

    Position_Index inline getA1PosIndex(void) const {return agent1PositionIndex;}
    Position_Index inline getA2PosIndex(void) const {return agent2PositionIndex;}

    unsigned int inline getDepth(void) const {return stepDepth;}

    const DynamicBitset& getBitset(void) const {return bitset;}

    inline const RecomputeState* getParent(void) const {return parentState;}
};

bool operator<(const RecomputeState& lhs, const RecomputeState& rhs);

bool operator==(const RecomputeState& lhs, const RecomputeState& rhs);

std::ostream &operator<<(std::ostream& os, const RecomputeState& rs);