#pragma once

#include "pch.h"

#include <vector>

#include "UtilLib/DynamicBitset.h"
#include "GCodeParser.h"

//TODO - make a parameter:
#define CHAIN_MIN_SEPERATION_MM 25.0

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

    inline unsigned int at(unsigned int i) const {
        return ((traversalDirection == Direction::FORWARD) ? 
                    startIndex + i : startIndex - i
        );
    }
};

struct PreComputeChain{
public:
    const Chain& c1;
    const Chain& c2;
    const unsigned int amountPrinted;

    PreComputeChain(const Chain& c1, const Chain& c2,
        const unsigned int amountPrinted) : c1(c1), c2(c2),
        amountPrinted(amountPrinted)
    {}
};

std::ostream& operator<<(std::ostream& os, const Chain& c);

typedef std::vector<Chain> ChainList;
typedef std::pair<unsigned int, unsigned int> UIntPair;

//stores metadata about a layer in the 
class ChainLayerMeta{
protected:
    // stores a list of all the chains in this layer
    ChainList chains;

    // tranlate a layer index to proper GCP index
    std::vector<unsigned int> segmentTranslation;

    // total print segments
    unsigned int totalPrintSegments;

    //ref to the GCP
    const GCodeParser& gcp;

    //which zLayer is this meta for
    const double zLayer;
public:
//constructors
    ChainLayerMeta(const GCodeParser& gcp, const double zLayer);

//methods

    //return a dynamic bitset that represents this chain in the layer
    //  (the bitset size is that of total print segments
    //   all elements in the chain are set, while others are not)
    DynamicBitset chainAsBitMask(unsigned int chainID) const;

    //determine how far the two chains can be printed before colliding
    // return how many steps can be completed concurrently
    unsigned int resolveChainPair(const Chain& chainA, const Chain& chainB) const;

//accessors

    inline const ChainList& getChainListRef(void) const {return chains;}
    inline unsigned int getNumChainsInLayer(void) const {return chains.size();}
    inline unsigned int getNumPrintSegmentsInLayer(void) const {
        return totalPrintSegments;
    }

//DEBUG functions

    //do an all pairs comparison, mostly just to see total time required
    void doAllPairsCheck(void) const;

    //check all the chains against each other
    void doAllChainsCheck(void) const;
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