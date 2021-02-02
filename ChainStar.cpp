#include "ChainStar.h"

void ChainStar::doRecomputeLayer(const GCodeParser& gcp, const double zLayer){
    //TODO
}

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
    chains(), segmentTranslation(), totalPrintSegments(0)
{
    const unsigned int layerStartIndex = gcp.getLayerStartIndex(layer);
    const unsigned int layerEndIndex = gcp.getLayerEndIndex(layer);

    bool inPrintChain = false;
    unsigned int printChainStart = 0;

    //loop over the print segments, adding chains as necessary
    //  do layerEnd+1 and for the else block to run at the end
    for(auto i = layerStartIndex; i <= layerEndIndex + 1; i++){
        bool isPrint = (i == (layerEndIndex + 1) ? 
            false : gcp.at(i).isPrintSegment()
        );

        if(isPrint){
            //update data about this being a print structure
            totalPrintSegments++;
            segmentTranslation.push_back(i);

            if(!inPrintChain){
                //the start of a new chain, so
                inPrintChain = true;
                printChainStart = i;
            }
        }else{
            if(inPrintChain){
                //this is the end of the print chain
                // time to add that information to the chains
                unsigned int chainLen = i - printChainStart;
                chains.emplace_back(printChainStart, chainLen, true);
                chains.emplace_back(i-1, chainLen, false); // the reverse chain
                inPrintChain = false;
            }
        }
    }
}