#pragma once

#include "pch.h"

#include "GCodeParser.h"
#include "GeometryLib/Point3.h"
#include "RecomputeState.h"


// do a pruned AStar Search on a single layer
// return type TBD, but probably a pair of positions where the agents ended up
// Parameters
//  gcp - the parsed GCODE object
//  layerStartInd - the index of the first segment that is part of this layer
//  layerEndInd - the index of the last segment that is part of this layer
void prunedAStarLayer(const GCodeParser& gcp, unsigned int layerStartInd, unsigned int layerEndInd);


// do a pruned AStar Search for each layer
// return type TBD
void prunedAStar(const GCodeParser& gcp);

// return true iff these two points form a valid position
inline bool isValidPositionPair(const Point3& p1, const Point3& p2){
    return getPointDistance(p1, p2) >= 25;
}