#pragma once

#include "pch.h"

#include <tuple>
#include <vector>

#include "ChainStarHelper.h"
#include "ChainLayerMeta.h"
#include "UtilLib/DynamicBitset.h"
#include "GCodeParser.h"


// #define SINGLE_LAYER_ONLY
// #warning "Forcing single layer mode"

//used to prevent wasting time on a bunch of small chains
#define CONCURRENT_CONSIDERATIONS_TARGET 100


//tuple ordering
//  chain 1 position
//  chain 2 position
//  PCC_Pair index (because you cant transition from self to self)
typedef std::tuple<Point3, Point3, unsigned int> PositionPair;

//tuple ordering
//  transition cost
//  PCC_pair transition start index
//  PCC_pair transition end index
typedef std::tuple<unsigned int, unsigned int, unsigned int> ResolvedTransition;

//tuple ordering
//  newTime - new time cost for layer
//  baseTime - original print time cost for layer
//  rawTime - original total time for layer
//  newBase - new print time cost for layer
typedef std::tuple<unsigned int, unsigned int, unsigned int, unsigned int>
    LayerResults;

//manages the recomputing of a file
class ChainStar{
protected:

    //run recompute on the specific layer
    LayerResults doRecomputeLayer(const GCodeParser& gcp, const double zLayer);

public:
    ChainStar(){};

    //recompute on the provided GCP file
    //  define pre-processor flag 'SINGLE_LAYER_ONLY' to run just the first layer
    //TODO - return type?
    void doRecompute(const GCodeParser& gcp);
};