#pragma once

#include "pch.h"

#include <tuple>
#include <vector>

#include "ChainStarHelper.h"
#include "ChainStarLog.h"
#include "ChainLayerMeta.h"
#include "UtilLib/DynamicBitset.h"
#include "GCodeLib/GCodeParser.h"

#include "RecomputeFrameworks.h"


// #define SINGLE_LAYER_ONLY
// #warning "Forcing single layer mode"

//used to prevent wasting time on a bunch of small chains
#ifndef CONCURRENT_CONSIDERATIONS_TARGET
#define CONCURRENT_CONSIDERATIONS_TARGET 100
#endif


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
//  transition cost
//  state a index
//  state b index
typedef std::tuple<unsigned int, unsigned int, unsigned int>
    StateTransition;

//tuple ordering
//  newTime - new time cost for layer
//  baseTime - original print time cost for layer
//  rawTime - original total time for layer
//  newBase - new print time cost for layer
typedef std::tuple<unsigned int, unsigned int, unsigned int, unsigned int>
    LayerResults;

//manages the recomputing of a file
class ChainStar{

public:
//subclasses
    enum class RecomputeMode {THEORETICAL, CODEX, CURRENT};

protected:
    struct State{
    public:
        Point3 a1Pos;
        Point3 a2Pos;
        unsigned int partnerIndex;
        unsigned int chainGroup;
        bool hasBeenUsed;
    };

    //run recompute on the specific layer
    LayerResults doRecomputeLayer(const GCodeParser& gcp, const double zLayer, const RecomputeMode mode);

    //do the phase 1 layer recompute:
    //  Matching segment chains together into pairs
    //      using CLM functions to determine if the chains can be concurrent
    //      specifically, the resolveChainPairs function
    void doPhase1LayerRecompute(
        std::vector<PreComputeChain>& resolvedPrecomputeChainPairs,
        const ChainLayerMeta& clm
    );

    //do the phase 2 layer recompute:
    //  Linking endpoints of chain pairs
    unsigned int doPhase2LayerRecompute(
        std::vector<PreComputeChain>& resolvedPrecomputeChainPairs,
        const ChainLayerMeta& clm
    );

public:

//constructors
    ChainStar(){};

//methods
    //recompute on the provided GCP file
    //  define pre-processor flag 'SINGLE_LAYER_ONLY' to run just the first layer
    //TODO - return type?
    void doRecompute(const GCodeParser& gcp, const RecomputeMode mode);
};