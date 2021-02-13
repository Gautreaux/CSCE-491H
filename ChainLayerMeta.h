#pragma once

#include "pch.h"

#include <algorithm> // for sort
#include <utility> // for pair
#include <vector>

#include "ChainStarHelper.h"
#include "UtilLib/DynamicBitset.h"
#include "GCodeParser.h"

typedef std::vector<Chain> ChainList;
typedef std::pair<unsigned int, unsigned int> UIntPair;

//TODO - make a parameter:
#define CHAIN_MIN_SEPERATION_MM 25.0

//stores metadata about a layer in the 
class ChainLayerMeta{
protected:
    // stores a list of all the chains in this layer sorted smallest to largest
    ChainList chains;

    // tranlate a layer index to proper GCP index
    std::vector<unsigned int> segmentTranslation;

    // total print segments in layer
    unsigned int totalPrintSegments;

    // total segments in layer
    unsigned int totalSegments;

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

    //return true iff this agent can print this segment
    bool canAgentAPrintSegment(const GCodeSegment& seg) const {
        return true;
    }


    //return true iff this agent can print this segment
    bool canAgentBPrintSegment(const GCodeSegment& seg) const {
        return true;
    }

    //return true iff this agent can print the segment at index i
    inline bool canAgentAPrintSegmentIndex(const unsigned int i) const {
        return canAgentAPrintSegment(gcp.at(segmentTranslation.at(i)));
    }
    
    //return true iff this agent can print the segment at index i
    inline bool canAgentBPrintSegmentIndex(const unsigned int i) const {
        return canAgentBPrintSegment(gcp.at(segmentTranslation.at(i)));
    }

    // unsigned int getPenaltyForTime()

//accessors

    inline const ChainList& getChainListRef(void) const {return chains;}
    inline unsigned int getNumChainsInLayer(void) const {return chains.size();}
    inline unsigned int getNumPrintSegmentsInLayer(void) const {
        return totalPrintSegments;
    }
    inline unsigned int getNumSegmentsInLayer(void) const {
        return totalSegments;
    }

//DEBUG functions

    //do an all pairs comparison, mostly just to see total time required
    void doAllPairsCheck(void) const;

    //check all the chains against each other
    void doAllChainsCheck(void) const;
};