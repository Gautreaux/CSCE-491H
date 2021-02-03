#include "ChainStar.h"

#include <PriorityQueue.h>

void ChainStar::doRecompute(const GCodeParser& gcp){
    assert(gcp.isValid());

    for(const double layer : gcp.getLayerVecRef()){
        //TODO - some return type stuff here?
        doRecomputeLayer(gcp, layer);
#ifdef SINGLE_LAYER_ONLY
        break;
#endif
    }
}

Chain::Chain(const unsigned int startIndex, const unsigned int chainLength, 
    const bool isForwardChain) : startIndex(startIndex), chainLength(chainLength),
    traversalDirection((isForwardChain ? 
        Chain::Direction::FORWARD : Chain::Direction::BACKWARD)
    )
{}

Chain::Chain(const Chain& other) : startIndex(other.startIndex),
    chainLength(other.chainLength), traversalDirection(other.traversalDirection)
{}

std::ostream& operator<<(std::ostream& os, const Chain& c){
    os << "C{";
    os << c.getStartIndex() << " " << c.getChainLength() << " ";
    os << (c.getDirection() == Chain::Direction::FORWARD ? "F" : "B");
    os << "}";

    return os;
}

ChainLayerMeta::ChainLayerMeta(const GCodeParser& gcp, const double layer) :
    chains(), segmentTranslation(), totalPrintSegments(0), gcp(gcp), zLayer(layer)
{
    const unsigned int layerStartIndex = gcp.getLayerStartIndex(layer);
    const unsigned int layerEndIndex = gcp.getLayerEndIndex(layer);

    bool inPrintChain = false;
    unsigned int printChainStart = 0;

    //loop over the print segments, adding chains as necessary
    for(auto i = layerStartIndex; i <= layerEndIndex; i++){
        if(gcp.at(i).isPrintSegment()){
            //update data about this being a print structure
            totalPrintSegments++;
            segmentTranslation.push_back(i);

            if(!inPrintChain){
                //the start of a new chain, so
                inPrintChain = true;
                printChainStart = totalPrintSegments-1;
            }
        }else{
            if(inPrintChain){
                //this is the end of the print chain
                // time to add that information to the chains
                unsigned int chainLen = (totalPrintSegments) - printChainStart;
                chains.emplace_back(printChainStart, chainLen, true);
                chains.emplace_back(totalPrintSegments - 1, chainLen, false); 
                inPrintChain = false;
            }
        }
    }

    if(inPrintChain){
        unsigned int chainLen = totalPrintSegments - printChainStart;
        chains.emplace_back(printChainStart, chainLen, true);
        chains.emplace_back(totalPrintSegments - 1, chainLen, false); 
        inPrintChain = false;
    }
}

unsigned int ChainLayerMeta::resolveChainPair(const Chain& chainA, 
    const Chain& chainB) const
{
    unsigned int maxLen = std::min(chainA.getChainLength(), chainB.getChainLength());
    for(unsigned int i = 0; i < maxLen; i++){
        if(
            gcp.at(segmentTranslation.at(chainA.at(i))).minSeperationDistance(
            gcp.at(segmentTranslation.at(chainB.at(i)))) < CHAIN_MIN_SEPERATION_MM
        ){
            return i;
        }
    }
    return maxLen;
}


// just to test exactly how bad doing an all pairs would be
//  turns out C++ is much more efficient than python (~250k seg pairs per sec)
//  and that it is not too bad to do this
void ChainLayerMeta::doAllPairsCheck(void) const {
    printf("Total concurrent starting.\n");
    unsigned int totalConcurrent = 0;
    for(unsigned int i : segmentTranslation){
        for(unsigned int j : segmentTranslation){
            // if(i == j) {continue;}
            if(gcp.at(i).minSeperationDistance(gcp.at(j)) >= 25){
                totalConcurrent++;
            }
        }
    }
    printf("Total concurrent finished, found %u segment pairs.\n", totalConcurrent);
}

void ChainLayerMeta::doAllChainsCheck(void) const {
    std::cout << "-";
    unsigned int chainNum = 0;
    while(chainNum < chains.size()){
        std::cout << " " << chainNum++;
    }
    std::cout << std::endl;
    chainNum = 0;

    for(const Chain& chain : chains){
        std::cout << chainNum++;
        for(const Chain& otherChain : chains){
            std::cout << " " << resolveChainPair(chain, otherChain);
        }
        std::cout << std::endl;
    }
}

