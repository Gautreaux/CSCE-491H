#pragma once

#include "pch.h"

#include "GCodeParser.h"
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