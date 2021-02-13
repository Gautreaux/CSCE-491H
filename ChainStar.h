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

//manages the recomputing of a file
class ChainStar{
protected:

    //run recompute on the specific layer
    void doRecomputeLayer(const GCodeParser& gcp, const double zLayer);

public:
    ChainStar(){};

    //recompute on the provided GCP file
    //  define pre-processor flag 'SINGLE_LAYER_ONLY' to run just the first layer
    //TODO - return type?
    void doRecompute(const GCodeParser& gcp);
};