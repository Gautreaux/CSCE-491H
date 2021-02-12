#pragma once

#include "pch.h"

#include <tuple>
#include <vector>

#include "UtilLib/DynamicBitset.h"
#include "GCodeParser.h"

//TODO - make a parameter:
#define CHAIN_MIN_SEPERATION_MM 25.0

#define SINGLE_LAYER_ONLY
#warning "Forcing single layer mode"

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
    Chain() : startIndex(0), chainLength(0), traversalDirection(Direction::FORWARD) {};
    Chain(const unsigned int startIndex, const unsigned int chainLength, 
        const bool isForwardChain);
    Chain(const Chain& other);
    // Chain& operator=(const Chain& other);

//methods

    //get the ending index of the chain
    //  note end index is inclusive on the interval
    inline unsigned int getEndIndex(void) const {
        return ((traversalDirection == Direction::FORWARD) ?
            startIndex + chainLength - 1 : startIndex - chainLength + 1
        );
    }


//accessors

    inline unsigned int getStartIndex(void) const {return startIndex;}
    inline unsigned int getChainLength(void) const {return chainLength;}
    inline Direction getDirection(void) const {return traversalDirection;}
    inline bool isForward(void) const {return traversalDirection == Direction::FORWARD;}

    inline unsigned int at(unsigned int i) const {
        return ((traversalDirection == Direction::FORWARD) ? 
                    startIndex + i : startIndex - i
        );
    }
};

struct PreComputeChain{
public:
    Chain c1;
    Chain c2;
    unsigned int amountPrinted;

    PreComputeChain() : c1(), c2(), amountPrinted(0) {};

    PreComputeChain(const Chain& c1, const Chain& c2,
        const unsigned int amountPrinted) : c1(c1), c2(c2),
        amountPrinted(amountPrinted)
    {}

    // Manual Definitions of various move/copy assignments/constructors
    // PreComputeChain(const PreComputeChain& other) : c1(other.c1), c2(other.c2),
    //     amountPrinted(other.amountPrinted)
    // {}
    //
    // PreComputeChain& operator=(const PreComputeChain& other){
    //     c1 = other.c1;
    //     c2 = other.c2;
    //     amountPrinted = other.amountPrinted;
    // }
    //
    // PreComputeChain(PreComputeChain&& other) : c1(other.c1), c2(other.c2),
    //     amountPrinted(other.amountPrinted)
    // {}
};

bool operator<(const Chain& lhs, const Chain& rhs);
bool operator<(const PreComputeChain& lhs, const PreComputeChain& rhs);

std::ostream& operator<<(std::ostream& os, const Chain& c);

typedef std::vector<Chain> ChainList;
typedef std::pair<unsigned int, unsigned int> UIntPair;

//stores metadata about a layer in the 
class ChainLayerMeta{
protected:
    // stores a list of all the chains in this layer sorted smallest to largest
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
    DynamicBitset chainAsBitMask(const Chain& c) const;

    //chainAsBitMask for both chains in the PreComputeChain
    DynamicBitset preComputeChainAsBitMask(const PreComputeChain& pcc) const;

    //determine how far along this chain agentA can traverse
    //  return how many steps that is
    //  presently, mostly insignificant function
    unsigned int resolveCainAgentA(const Chain& chain);

    //determine how far along this chain agentB can traverse
    //  return how many steps that is
    //  presently, mostly insignificant function
    unsigned int resolveChainAgentB(const Chain& chain);

    //determine how far the two chains can be printed before colliding
    // return how many steps can be completed concurrently
    unsigned int resolveChainPair(const Chain& chainA, const Chain& chainB) const;
    inline unsigned int resolveChainPair(
        const std::pair<const Chain&, const Chain&> chainPair) const 
    {
        return resolveChainPair(chainPair.first, chainPair.second);
    }

    inline Point3 getChainStartPoint(const Chain& chain) const {
        const GCodeSegment& seg = gcp.at(chain.getStartIndex());
        return ((chain.isForward()) ? seg.getStartPoint() : seg.getEndPoint());
    }

    inline Point3 getChainEndPoint(const Chain& chain) const {
        const GCodeSegment& seg = gcp.at(chain.getEndIndex());
        return ((chain.isForward()) ? seg.getEndPoint() : seg.getStartPoint());
    }
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
    ChainStar(){};

    //recompute on the provided GCP file
    //  define pre-processor flag 'SINGLE_LAYER_ONLY' to run just the first layer
    //TODO - return type?
    void doRecompute(const GCodeParser& gcp);
};