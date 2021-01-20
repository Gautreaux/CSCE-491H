#pragma once

#include "pch.h"

#include "algorithm"
#include <queue>
#include <set>
#include <vector>

#include "GCodeParser.h"
#include "GeometryLib/Point3.h"
#include "LayerManager.h"
#include "PQWrapper.h"
#include "RecomputeState.h"
#include "UtilLib/BiMap.h"
#include "UtilLib/DynamicBitset.h"
#include "UtilLib/NonReallocVector.h"
// #include "UtilLib/PriorityQueue.h"

#ifndef LOOP_PRINT_FREQUENCY
#ifdef DEBUG_1
#define LOOP_PRINT_FREQUENCY 1
#else
#define LOOP_PRINT_FREQUENCY 1000000
#endif
#endif

// maximum number of print segments allowed in a step back 
#define MAX_STEPBACK_ALLOWED 10000

//minimum acceptable efficiency of the solution
#define EFFICIENCY_TARGET 1.8


typedef BiMap<Point3, unsigned int> PosIndexBiMap;
typedef std::vector<std::vector<unsigned int>> PosSegMap;

//compare types for a quick change of whats going on
//compare using the default < operator
typedef std::less<RecomputeState> DefaultCompare;

//compare based on greatest depth (or is this least depth)
//  greatest depth expanded first
struct DepthCompare {
public:
    bool operator()(const RecomputeState& lhs, const RecomputeState& rhs){
        if(lhs.getDepth() < rhs.getDepth()){
            return true;
        }else if(lhs.getDepth() > rhs.getDepth()){
            return false;
        }
        return lhs < rhs;
    }
};

//compare on number of unset bits
//  least unset bits (most set bits) expanded first
struct SetBitCompare {
public:
    bool operator()(const RecomputeState& lhs, const RecomputeState& rhs){
        return lhs.getBitset().getUnsetCount() > rhs.getBitset().getUnsetCount();
    }
};

// compare based on a more intelligent compare
//  balances depth vs total states expanded
struct PriorityCompare {
public:
    bool operator()(const RecomputeState& lhs, const RecomputeState& rhs){
        //                      discount version of math::ceil(lhs.getUnsetCount() / 2.0);
        auto lhsCost = lhs.getDepth() + ((lhs.getBitset().getUnsetCount()-1) / 2) + 1;
        auto rhsCost = rhs.getDepth() + ((rhs.getBitset().getUnsetCount()-1) / 2) + 1;
        return lhsCost < rhsCost;
    }
};

struct Priority2Compare {
public:
    bool operator()(const RecomputeState& lhs, const RecomputeState& rhs){
        if(lhs.getEfficiency() < rhs.getEfficiency()){
            return true;
        }else if(lhs.getEfficiency() > rhs.getEfficiency()){
            return false;
        }
        if(lhs.getDepth() < rhs.getDepth()){
            return true;
        }else if(lhs.getDepth() > rhs.getDepth()){
            return false;
        }
        return lhs < rhs;
    }
};


struct Priority3Compare{
public:
    bool operator()(const RecomputeState& lhs, const RecomputeState& rhs){
        return false;
    }
};

// typedef std::priority_queue<RecomputeState, std::vector<RecomputeState>, DepthCompare> State_PQ;
typedef PQWrapper<DepthCompare> State_PQ;


struct StatePointerCompare{
    bool operator() (const RecomputeState* lhs, const RecomputeState* rhs) const {
        return (*lhs) < (*rhs);
    }
};

typedef std::set<RecomputeState*, StatePointerCompare> State_Set;

#define COLLISION_TOLERANCE 25

// do a pruned AStar Search on a single layer
// return type TBD, but probably a pair of positions where the agents ended up
// Parameters
//  gcp - the parsed GCODE object
//  layerStartInd - the index of the first segment that is part of this layer
//  layerEndInd - the index of the last segment that is part of this layer
void prunedAStarLayer(const GCodeParser& gcp, double layer);


// do a pruned AStar Search for each layer
// return type TBD
void prunedAStar(const GCodeParser& gcp);

// return true iff these two points form a valid position
inline bool isValidPositionPair(const Point3& p1, const Point3& p2){
    return getPointDistance(p1, p2) >= COLLISION_TOLERANCE;
}

// return true if the two segments are far enough apart
//  also implies that the endpoints are at least this far apart
inline bool isValidSegmentsPair(const GCodeSegment& s1, const GCodeSegment& s2){
    return s1.minSeperationDistance(s2) >= COLLISION_TOLERANCE;
}

//return true if the segment can be traversed while
//  the other agent does a NO-op
//  two functions provided as the function may not be reflexive
inline bool isValidSegNOOP(const GCodeSegment& a1Segment, const Point3& a2Pos){
    return a1Segment.minSeperationDistance(a2Pos) >= COLLISION_TOLERANCE;
}
inline bool isValidSegNOOP(const Point3& a1Pos, const GCodeSegment& a2Segment){
    return a2Segment.minSeperationDistance(a1Pos) >= COLLISION_TOLERANCE;
}

//using the gcp and the lm, push all the items into the PriorityQueue
void generateStartingPositions(const GCodeParser& gcp, const LayerManager& lm, State_PQ& pq);

//print a more verbose representation of the state
void printVerbose(std::ostream& os, const RecomputeState& state, const GCodeParser& gcp, const LayerManager& lm);

// do the updating of the search states
//typdef the function pointer for state generators
typedef void(*StateGeneratorFunctionPtr)(const RecomputeState*, const GCodeParser&, const LayerManager&, State_PQ&);
#define QTY_STATE_GENERATORS 4

//add all the dual print moves to the queue
void updateSearchStatesDualPrints(
    const RecomputeState* state, const GCodeParser& gcp,
    const LayerManager& lm, State_PQ& pq);

//add all the print noop states to the queue
void updateSearchStatesSinglePrintNoOp(
    const RecomputeState* state, const GCodeParser& gcp,
    const LayerManager& lm, State_PQ& pq);

//add all the print single move to the queue
void updateSearchStatesSinglePrintMove(
    const RecomputeState* state, const GCodeParser& gcp,
    const LayerManager& lm, State_PQ& pq);

//desperatelly add any possible state to the queue
//  hopefully never called
void updateSearchStatesRemainingStates(
    const RecomputeState* state, const GCodeParser& gcp,
    const LayerManager& lm, State_PQ& pq);