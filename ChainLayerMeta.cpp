#include "ChainLayerMeta.h"


unsigned int ChainLayerMeta::resolveChainPair(const Chain& chainA, 
    const Chain& chainB) const
{
    unsigned int maxLen = std::min(chainA.getChainLength(), chainB.getChainLength());
    for(unsigned int i = 0; i < maxLen; i++){
        if(!(canAgentAPrintSegmentIndex(chainA.at(i)) &&
                canAgentBPrintSegmentIndex(chainB.at(i)))){
            //one of the two agents cannot print its assigned segment
            //  so just stop and return
            return i;
        }

        if(
            canMoveSegmentPair(gcp.at(segmentTranslation.at(chainA.at(i))),
            gcp.at(segmentTranslation.at(chainB.at(i))), true, true) == false
        ){
            //we found the first non-printable
            //  so return the number of segments printed
            return i;
        }
    }
    return maxLen;
}

DynamicBitset ChainLayerMeta::chainAsBitMask(const Chain& c) const {
    DynamicBitset dbs(getNumPrintSegmentsInLayer());

    unsigned int start = c.getStartIndex();
    unsigned int end = ((c.getDirection() == Chain::Direction::FORWARD) ?
        start + c.getChainLength() - 1 : (start - c.getChainLength()) + 1
    );
    if(c.getDirection() == Chain::Direction::BACKWARD) {std::swap(start, end);}

    for(unsigned int j = start; j <= end; j++){
        dbs.set(j, true);
    }

    return dbs;
}

DynamicBitset ChainLayerMeta::preComputeChainAsBitMask(const PreComputeChain& pcc) const {
    DynamicBitset dbs(getNumPrintSegmentsInLayer());

    for(int i = 0; i < 2; i++){
        const Chain& c = ((i == 0) ? pcc.c1 : pcc.c2);

        unsigned int start = c.getStartIndex();
        unsigned int end = ((c.getDirection() == Chain::Direction::FORWARD) ?
            start + pcc.amountPrinted - 1 : (start - pcc.amountPrinted) + 1
        );
        if(c.getDirection() == Chain::Direction::BACKWARD) {std::swap(start, end);}

        for(unsigned int j = start; j <= end; j++){
            dbs.set(j, true);
        }
    }

    return dbs;
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

//prints a grid of how the chains are resolved against eachother
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

unsigned int ChainLayerMeta::getTransitionTime(
    const Point3& a1p1, const Point3& a1p2,
    const Point3& a2p1, const Point3& a2p2
) const {
    const LineSegment a1Seg(a1p1, a2p2);
    const LineSegment a2Seg(a2p1, a2p2);
    const double d1 = ceil(a1Seg.length() / CHAIN_MIN_SEPERATION_MM);
    const double d2 = ceil(a2Seg.length() / CHAIN_MIN_SEPERATION_MM);
    if (canMoveSegmentPair(a1Seg, a2Seg, false, false))
    {
        return (unsigned int)std::max(d1, d2);
    }

    if(isValidSegmentPosition(a1Seg, a2p1) && isValidSegmentPosition(a1p2, a2Seg)){
        //a1 moves then a2 moves
        //no added offsets
        return (unsigned int)(d1 + d2);
    }else if(isValidSegmentPosition(a1Seg, a2p2) && isValidSegmentPosition(a1p1, a2Seg)){
        //a2 moves then a1 moves
        //no added offsets
        return (unsigned int)(d1 + d2);
    }

    //there is a potentially +2 bound where only one agent needs to offset
    //but for the sake of ease we just overestimate
    //  and let each agent move away and back
    return (unsigned int)(d1 + d2) + 4;
}


ChainLayerMeta::ChainLayerMeta(const GCodeParser& gcp, const double layer) :
    chains(), segmentTranslation(), totalPrintSegments(0), totalSegments(0),
    gcp(gcp), zLayer(layer)
{
    const unsigned int layerStartIndex = gcp.getLayerStartIndex(layer);
    const unsigned int layerEndIndex = gcp.getLayerEndIndex(layer);

    bool inPrintChain = false;
    unsigned int printChainStart = 0;

    //loop over the print segments, adding chains as necessary
    for(auto i = layerStartIndex; i <= layerEndIndex; i++){
        totalSegments++;
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

    std::sort(chains.begin(), chains.end());
}

ChainLayerMeta::~ChainLayerMeta(void){};