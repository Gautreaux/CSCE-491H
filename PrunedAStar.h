#pragma once

#include "pch.h"

#include "algorithm"
#include <queue>
#include <set>
#include <vector>

#include "GCodeParser.h"
#include "GeometryLib/Point3.h"
#include "LayerManager.h"
#include "RecomputeState.h"
#include "UtilLib/BiMap.h"
#include "UtilLib/DynamicBitset.h"
#include "UtilLib/NonReallocVector.h"
// #include "UtilLib/PriorityQueue.h"

typedef BiMap<Point3, unsigned int> PosIndexBiMap;
typedef std::vector<std::vector<unsigned int>> PosSegMap;

//compare types for a quick change of whats going on
typedef std::less<RecomputeState> DefaultCompare;
struct DepthCompare {
public:
    bool operator()(const RecomputeState& lhs, const RecomputeState& rhs){
        return lhs.getBitset().getUnsetCount() > rhs.getBitset().getUnsetCount();
        // if(lhs.getDepth() < rhs.getDepth()){
        //     return true;
        // }else if(lhs.getDepth() > rhs.getDepth()){
        //     return false;
        // }
        // return lhs < rhs;
    }
};
typedef std::priority_queue<RecomputeState, std::vector<RecomputeState>, DepthCompare> State_PQ;


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

// do the updating of the search states
void updateSearchStates(
    const RecomputeState* state, const GCodeParser& gcp,
    const LayerManager& lm, State_PQ& pq);

//using the gcp and the lm, push all the items into the PriorityQueue
void generateStartingPositions(const GCodeParser& gcp, const LayerManager& lm, State_PQ& pq);

//print a more verbose representation of the state
void printVerbose(std::ostream& os, const RecomputeState& state, const GCodeParser& gcp, const LayerManager& lm);