#include "ChainStar.h"

#include <algorithm>
#include <functional>
#include <set>
#include <tuple>
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

    std::sort(chains.begin(), chains.end());
}

//TODO - this should be in the ChainLayerMeta scope?
unsigned int resolveCainAgentA(const Chain& chain){
    //TODO - add a check if agentA can print the whole chain
    //since right now the chains are interchangable, just return length
    return chain.getChainLength();
}

//TODO - this should be in the ChainLayerMeta scope?
unsigned int resolveChainAgentB(const Chain& chain){
    //TODO - add a check if agentB can print the whole chain
    //since right now the chains are interchangable, just return length
    return chain.getChainLength();
}


unsigned int ChainLayerMeta::resolveChainPair(const Chain& chainA, 
    const Chain& chainB) const
{
    unsigned int maxLen = std::min(chainA.getChainLength(), chainB.getChainLength());
    for(unsigned int i = 0; i < maxLen; i++){
        if(
            //TODO - add a check if chainA.at(i) can be printed by agentA
            //TODO - add a check if chainB.at(i) can be printed by agentB
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

bool operator<(const Chain& lhs, const Chain& rhs){

    if(lhs.getChainLength() < rhs.getChainLength()){
        return true;
    }else if(lhs.getChainLength() > rhs.getChainLength()){
        return false;
    }    
    
    if(lhs.getStartIndex() < rhs.getStartIndex()){
        return true;
    }else if(lhs.getStartIndex() > rhs.getStartIndex()){
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

//used to prevent wasting time on a bunch of small chains
#define CONCURRENT_CONSIDERATIONS_TARGET 100

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

    //iterator for the chain source
    auto chainSourceIter = clm.getChainListRef().rbegin();
    auto chainSourceEnd = clm.getChainListRef().rend();

    //stores the pairs for the precompute chain that are resolved
    std::vector<PreComputeChain> resolvedPrecomputeChainPairs;

    while(true){
        if(currentPrinted.getUnsetCount() == 0){
            //we have pathed the whole layer
            break;
        }

        //backfill the concurrent considerations
        while((chainSourceIter != chainSourceEnd) &&
                (consideredChains.size() < CONCURRENT_CONSIDERATIONS_TARGET))
        {
            consideredChains.insert(*chainSourceIter);
            chainSourceIter++;
        }        
        
        if(currentPrinted.getSetCount() == 0){
            //first iteration, all chains are pending
            //do nothing
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

        // (Deprecated) remove all chains that are below the threashold
        // unsigned int minChainActual;
        // if((*consideredChains.end()).getChainLength() <= MIN_CHAIN_THRESHOLD){
        //     minChainActual = 0;
        // }else{
        //     minChainActual = MIN_CHAIN_THRESHOLD;
        // }
        // Chain testC(0, minChainActual, true);
        // auto eIter = consideredChains.upper_bound(testC);
        // consideredChains.erase(consideredChains.begin(), eIter);

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

        if(validPCC){
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

            //construct a new pcc with only printed chains
            PreComputeChain pcc_new(
                Chain(pcc.c1.getStartIndex(), pcc.amountPrinted, pcc.c1.isForward()),
                Chain(pcc.c2.getStartIndex(), pcc.amountPrinted, pcc.c2.isForward()),
                pcc.amountPrinted
            );

            //store this pre-compute-chain for the second phase
            resolvedPrecomputeChainPairs.push_back(pcc_new);
            continue;
        }

        //this is a non-valid pcc,
#ifdef DEBUG //some debug checking and printing
        printf("All dual move segments resolved.\n");
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

        if(chainSourceIter != chainSourceEnd){
            printf("WARNING! not all source chains consumed\n");
        }else{
            printf("All source chains consumed\n");
        }
        
        //check if the following are both true
        //  1. the remaining chains covers all the printed segments
        //  2. the remaining chains do not overlap at all;
        DynamicBitset partialDBS(currentPrinted);
        unsigned int partialSum = 0;

        for(const Chain& chain1 : consideredChains){
            partialDBS = partialDBS | clm.chainAsBitMask(chain1);
            partialSum += chain1.getChainLength();
        }

        if(partialDBS.getUnsetCount() == 0){
            printf("Remaining chains fully cover layer\n");
        }else{
            printf("WARNING! remaining chains do not fully cover layer.\n");
        }

        if(partialSum == (partialDBS.getSetCount() - currentPrinted.getSetCount())){
            printf("Remaning chains have no overlap.\n");
        }else{
            printf("%u/%u overlap in the remaning chains set.\n", 
                partialSum, currentPrinted.getUnsetCount());
        }
#endif

        //TODO - push final chains for full coverages
        Chain noopChain(0, 0, true);

        while(currentPrinted.getUnsetCount() > 0){
            if(consideredChains.size() == 0){
                throw std::runtime_error(
                        "Could not utilize remainder chains to cover unprinted"
                );
            }

            //since chain is ordered in-order,
            //  this is the longest chain
            const Chain c = *(consideredChains.rbegin());
            consideredChains.erase(c);
            DynamicBitset dbs = clm.chainAsBitMask(c);

            //TODO - or sum is more efficient
            if((dbs & currentPrinted).getSetCount() != 0){
                //there was overlap in chain and printed
                continue;
            }

            //TODO - check which agent(s) can print this chain

            PreComputeChain pccNew(c, noopChain, c.getChainLength());
            currentPrinted = currentPrinted | dbs; // TODO - overload |=
            resolvedPrecomputeChainPairs.push_back(pccNew);
        }

        //since we now have full coverage:
        break; //would happen at top of loop anyway but whatever
    } // while(True)

    //debug checking to ensure this is a valid config
    //TODO - move into a preprocessor block?
    printf("Starting pre-phase 2 checks\n");
    printf("Total chain pairs: %lu\n", resolvedPrecomputeChainPairs.size());
    DynamicBitset debugDBS(clm.getNumPrintSegmentsInLayer());
    unsigned int numOverlapping = 0;
    for(const PreComputeChain& pcc : resolvedPrecomputeChainPairs){
        for(int i = 0; i < 2; i++){
            const Chain& origChainRef = ((i == 0) ? pcc.c1 : pcc.c2);
            if(origChainRef.getChainLength() == 0){
                continue;
            }
            
            DynamicBitset dbs = clm.chainAsBitMask(origChainRef);
            if((dbs & debugDBS).getSetCount() != 0){
                std::cout << "Chain overlaps already printed: " << origChainRef << std::endl;
                numOverlapping++;
            }
            debugDBS = debugDBS | dbs;
        }
    }

    if(numOverlapping != 0){
        throw std::runtime_error("Error in pre-phase 2 checks, double-printed segments\n");
    }
    if(debugDBS.getUnsetCount() != 0){
        throw std::runtime_error("Error in pre-phase 2 checks, unprinted segments\n");
    }
    std::cout << "Passed pre-phase 2 checks." << std::endl;

    //now link partial chains together in an efficient way
    //  start with the set of all chains
    //  while the size of the set of all chains > 1:
    //      determine the time from each chain endpoint
    //          to all other chain endpoints
    //      pick the closest two chain's endpoints
    //      remove corresponding chains from set
    //      combine the two chains into a super chain
    //      insert new chain into set
    // in practice its a little more complicated

    //stores all the positions of chains that we need to deal with
    //TODO - tuple into a struct
    //tuple ordering
    //  chain 1 position
    //  chain 2 position
    //  PCC_Pair index (because you cant transition from self to self)
    std::vector<std::tuple<Point3, Point3, unsigned int> > chainPositions;

    //TODO - PROBLEM IN HERE
    //  HOW TO HANDLE THE NOOP CHAINS?
    //  THE POINTS THAT ARE ALL ZERO
    //now we need to add the endpoints to that set
    for(unsigned int i = 0; i < resolvedPrecomputeChainPairs.size(); i++){
        const PreComputeChain& pcc = resolvedPrecomputeChainPairs[i];
        auto t = std::tuple<Point3, Point3, unsigned int>(
            clm.getChainStartPoint(pcc.c1),
            clm.getChainStartPoint(pcc.c2),
            i
        );

        chainPositions.push_back(t);

        t = std::tuple<Point3, Point3, unsigned int>(
            clm.getChainEndPoint(pcc.c1),
            clm.getChainEndPoint(pcc.c2),
            i
        );

        chainPositions.push_back(t);
    }

    //TODO - tuple into struct
    //tuple ordering
    //  transition cost
    //  PCC_pair transition start index
    //  PCC_pair transition end index
    std::priority_queue<std::tuple<unsigned int, unsigned int, unsigned int> >
        transitionsPQ;

    //for some position pair starting at i and ending at j
    //  calculate the cost to transition from i to j
    for(unsigned int i = 0; i < chainPositions.size(); i++){
        for(unsigned int j = 0; j < chainPositions.size(); j++){
            if(std::get<2>(chainPositions.at(i)) == 
                std::get<2>(chainPositions.at(j)))
            {
                //these are the two endpoints of the same chain
                //  (or the same point if i == j)
                //so this is not a valid transition
                continue;
            }

            //TODO
            //calculate the transition time for this pair
            int transitionTime = 0;

            auto t = std::tuple<unsigned int, unsigned int, unsigned int>(
                transitionTime, i, j
            );
            transitionsPQ.push(t);
        }
    }

    //bitset for tracking which positions are (not) used already
    DynamicBitset poisitionsBitset(chainPositions.size());

    //tracks the total transition time penalty
    unsigned int totalTransitionsTime = 0;

    //compare against two, because two points
    //  the start and end of the layer
    //  can be unset and there is no problem
    //TODO - long term
    //  can inject a layer start point to make sure that the 
    //      recomputed layer starts at some location
    //      relatively reasonable to the previous layer's end point
    while(poisitionsBitset.getUnsetCount() != 2){
        if(poisitionsBitset.getUnsetCount() < 2){
            //pretty sure this is an error and unreachable
            std::cout << "Warning: reached possibly invalid state:"
                " <2 unset in transition" << std::endl;
            break;
        }

        while(true){
            auto t = transitionsPQ.top();
            transitionsPQ.pop();

            //check if this is a new transition and is needed
            unsigned int transition_start = std::get<1>(t);
            unsigned int transition_end = std::get<2>(t);

            if(poisitionsBitset.at(transition_start)){
                //the transition's start point was already used
                continue;
            }

            if(poisitionsBitset.at(transition_end)){
                //the transition's end point was already used
                continue;
            }

            poisitionsBitset.set(transition_start);
            poisitionsBitset.set(transition_end);
            totalTransitionsTime += std::get<0>(t);
        }
    }

    //debug info
    std::cout << "Total Transitions Time: " << totalTransitionsTime << std::endl;


    //debug check that the resulting path is
    // 1. a valid non-colliding path
    // 2. covers, with no extra segments, the original path

    //finally export to a reasonable representation
}