#pragma once

#include "pch.h"

#include <vector>

#include "DynamicBitset.h"
#include "GCodeParser.h"


//represents a single print chain
class Chain{
public:    
//subclasses
    enum class Direction {FORWARD, BACKWARD};

protected:
//data members

    //starting index for the chain
    unsigned int startIndex;

    //number of elements in the chain
    unsigned int chainLength;

    //direction the chain moves in the layer's provided ordering
    Direction traversalDirection;

public:
//constructors
    Chain(const unsigned int startIndex, const unsigned int chainLength, 
        const bool isForwardChain);
    Chain(const Chain& other);
    // Chain& operator=(const Chain& other);

//accessors

    inline unsigned int getStartIndex(void) const {return startIndex;}
    inline unsigned int getChainLength(void) const {return chainLength;}
    inline Direction getDirection(void) const {return traversalDirection;}
};

std::ostream& operator<<(std::ostream& os, const Chain& c);

typedef std::vector<Chain> ChainList;

//stores metadata about a layer in the 
class ChainLayerMeta{
protected:
    // stores a list of all the chains in this layer
    ChainList chains;

    // tranlate a layer index to proper GCP index
    std::vector<unsigned int> segmentTranslation;

    // total print segments
    unsigned int totalPrintSegments;
public:
//constructors
    ChainLayerMeta(const GCodeParser& gcp, const double zLayer);

//accessors

    inline const ChainList& getChainListRef(void) const {return chains;}
};

//manages the recomputing of a file
class ChainStar{
protected:

    //run recompute on the specific layer
    void doRecomputeLayer(const GCodeParser& gcp, const double zLayer);

public:
    ChainStar();

    //recompute on the provided GCP file
    //  define pre-processor flag 'SINGLE_LAYER_ONLY' to run just the first layer
    //TODO - return type?
    void doRecompute(const GCodeParser& gcp);
};