#pragma once

#include "pch.h"

#include <tuple>
#include <vector>

#include "ChainStarHelper.h"
#include "ChainStarLog.h"
#include "ChainLayerMeta.h"
#include "UtilLib/DynamicBitset.h"
#include "GCodeLib/GCodeParser.h"


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

    //determine the total transition time for the agents
    //  assumes that a1 transitions between a1p1 and a1p2 but 
    //  does not assume that a1p1 is the start of the transition
    //  same is true for a2
    unsigned int getTransitionTime(const Point3& a1p1, const Point3& a1p2,
        const Point3& a2p1, const Point3& a2p2, const ChainLayerMeta& clm) const;

public:
    ChainStar(){};

    //recompute on the provided GCP file
    //  define pre-processor flag 'SINGLE_LAYER_ONLY' to run just the first layer
    //TODO - return type?
    void doRecompute(const GCodeParser& gcp);
};