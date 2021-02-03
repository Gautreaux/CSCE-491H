#include "ChainStar.h"

#include <set>
#include <queue>

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

bool operator<(const Chain& lhs, const Chain& rhs){
    if(lhs.getStartIndex() < rhs.getStartIndex()){
        return true;
    }else if(lhs.getStartIndex() > rhs.getStartIndex()){
        return false;
    }

    if(lhs.getChainLength() < rhs.getChainLength()){
        return true;
    }else if(lhs.getChainLength() > rhs.getChainLength()){
        return false;
    }

    if(lhs.getDirection() == rhs.getDirection()){
        return false;
    }

    if(lhs.getDirection() == Chain::Direction::FORWARD){
        //implies rhs.getDirection() == Chain::Direction::Backwards
        return true;
    }else{
        //implies lhs.getDirection() == Chain::Direction::Backwards
        //implies rhs.getDirection() == Chain::Direction::Forwards
        return false;
    }
}

bool operator<(const PreComputeChain& lhs, const PreComputeChain& rhs){
    if(lhs.amountPrinted < rhs.amountPrinted){
        return true;
    }else if(lhs.amountPrinted > rhs.amountPrinted){
        return false;
    }

    if(lhs.c1 < rhs.c1){
        return true;
    }else if(rhs.c1 < lhs.c1){
        return false;
    }

    return lhs.c2 < rhs.c2;
}

void ChainStar::doRecomputeLayer(const GCodeParser& gcp, const double zLayer){
    const ChainLayerMeta clm(gcp, zLayer);
    const int numberSegmentsInLayer = clm.getNumPrintSegmentsInLayer();

    //tracks which segments are currently printed
    DynamicBitset currentPrinted(numberSegmentsInLayer);

    //stores which chain pairs we have already computed the path for
    std::set<std::pair<Chain, Chain>> cachedChainPairs;

    //stores the pending chain pairs
    std::priority_queue<PreComputeChain> cachedChainPairsPQ;

    //stores the chains currently in consideration
    std::set<Chain> consideredChains;

    //count the number of chain reductions performed
    unsigned int reductionRounds = 0;

    while(true){
        if(currentPrinted.getUnsetCount() == 0){
            //we have pathed the whole layer
            break;
        }

        if(currentPrinted.getSetCount() == 0){
            //first iteration, all chains are pending
            for(const Chain& chain : clm.getChainListRef()){
                consideredChains.insert(chain);
            }
        }
        else{
            //cut down the pending chains for anything that overlaps
            const std::set<Chain> oldChains = std::move(consideredChains);

            for(const Chain& oldChain : oldChains){
                // create the new chain(s) of portions that do not
                //  include any already-printed stuff
                // remove any chains from the set
                DynamicBitset dbsMask = clm.chainAsBitMask(oldChain);
                DynamicBitset dbsAND = (dbsMask & currentPrinted); 
                if(dbsAND.getSetCount() == 0){
                    //no overlap in the chain and dbsMask
                    consideredChains.insert(oldChain);
                }else{
                    //do not reinsert since no longer considered
                    //see if new remainder chains are formed
                    bool inRemainderRun = false;
                    unsigned int runLen = 0;
                    unsigned int runStart = 0;

                    unsigned int start = oldChain.getStartIndex();
                    unsigned int end = oldChain.getEndIndex();
                    if(end < start) {std::swap(start, end);}
                    for(unsigned int i = start; i <= end; i++){
                        if(currentPrinted.at(i) == false){
                            if(inRemainderRun == false){
                                inRemainderRun = true;
                                runStart = i;
                            }
                            runLen++;
                        }else{
                            if(inRemainderRun){
                                Chain c(runStart, runLen, true);
                                consideredChains.insert(c);
                                consideredChains.insert(
                                    Chain(c.getEndIndex(), runLen, false)
                                );
                                inRemainderRun = false;
                                runLen = 0;
                            }
                        }
                    }
                    if(inRemainderRun){
                        Chain c(runStart, runLen, true);
                        consideredChains.insert(c);
                        consideredChains.insert(
                            Chain(c.getEndIndex(), runLen, false)
                        );
                }
                }
            }
        }

        //ensure that all current chains have the collision-ness computed
        for(const Chain& chain1 : consideredChains){
            for(const Chain& chain2 : consideredChains){
                std::pair<Chain, Chain> chainPair(chain1, chain2);
                if(cachedChainPairs.find(chainPair) == cachedChainPairs.end()){
                    //this is a new chain pair, do the compute
                    const unsigned int i = clm.resolveChainPair(chainPair);
                    PreComputeChain pcc(chain1, chain2, i);
                    cachedChainPairsPQ.push(pcc);
                }
            }
        }

        PreComputeChain pcc;
        bool validPCC = false;
        while(cachedChainPairsPQ.size() > 0){
            pcc = cachedChainPairsPQ.top();
            cachedChainPairsPQ.pop();
            std::pair<Chain, Chain> chainPair(pcc.c1, pcc.c2);
            //erase this pair as it is no longer under consideration
            cachedChainPairs.erase(chainPair);

            //check if this is still a valid-pre cache
            //  no need to add the new segments, as
            //      the old chains loop above will take care of it
            if(consideredChains.find(pcc.c1) == consideredChains.end()){
                continue;
            }
            if(consideredChains.find(pcc.c2) == consideredChains.end()){
                continue;
            }
            if(pcc.amountPrinted == 0){
                //this will ultimately terminate into a runtime error
                //  because we order on amount printed first
                continue;
            }

            validPCC = true;
            break;
        }

        if(!validPCC){
            printf("Probable error occurred. Dumping Data\n");
            printf("Number of printed segments: %u/%u (unprinted: %u)\n",
                currentPrinted.getSetCount(), clm.getNumPrintSegmentsInLayer(), 
                currentPrinted.getUnsetCount()
            );
            printf("Number of pending chains: %lu\n", consideredChains.size());

            unsigned int longestChain = 0;
            for(const Chain& chain1 : consideredChains){
                if(chain1.getChainLength() > longestChain){
                    longestChain = chain1.getChainLength();
                }
            }
            printf("Longest Remaining Chain: %u\n", longestChain);

            //so lets check one thing: are there really no moves
            unsigned int longestPair = 0;
            for(const Chain& chain1 : consideredChains){
                for(const Chain& chain2 : consideredChains){
                    unsigned int i = clm.resolveChainPair(chain1, chain2);
                    if(i > longestPair){
                        longestPair = i;
                    }
                }
            }
            printf("Longest remaining pair: %u\n", longestPair);

            //TODO - what to do here
            printf("Error in Recomputing layer.\n");
            throw std::runtime_error("Invalid state possibly reached");
        }

        const DynamicBitset dbsMask = clm.preComputeChainAsBitMask(pcc);

        //debug check
        //TODO - remove
        assert((currentPrinted & dbsMask).getSetCount() == 0);

        //TODO can optimize here if really wanted to
        //  dont construct new bitset & copy, just update in place
        currentPrinted = currentPrinted | dbsMask;

        reductionRounds++;

        //log info about the progression
        printf("Reduction round %u: added %u*2 segments, total %u/%u, pending: %lu\n", 
            reductionRounds, pcc.amountPrinted, currentPrinted.getSetCount(),
            clm.getNumPrintSegmentsInLayer(), cachedChainPairsPQ.size()
        );
    }
}