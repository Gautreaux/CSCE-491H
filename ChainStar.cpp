#include "ChainStar.h"

#include <algorithm>
#include <functional>
#include <set>
#include <ctime>
#include <tuple>
#include <queue>

void ChainStar::doRecompute(const GCodeParser& gcp){
    assert(gcp.isValid());

    unsigned int newTimeSum = 0;
    unsigned int baseTimeSum = 0;
    unsigned int rawTimeSum = 0;
    unsigned int newBaseSum = 0;
    unsigned int layersRun = 0;

    clock_t startTime = clock();

    for(const double layer : gcp.getLayerVecRef()){
        //TODO - some return type stuff here?
        LayerResults r = doRecomputeLayer(gcp, layer);

        newTimeSum += std::get<0>(r);
        baseTimeSum += std::get<1>(r);
        rawTimeSum += std::get<2>(r);
        newBaseSum += std::get<3>(r);

        layersRun++;
#ifdef SINGLE_LAYER_ONLY
        break;
#endif
    }

    clock_t endTime = clock();
    printf("Recompute duration (s): %.3f\n", double(endTime-startTime)/CLOCKS_PER_SEC);

    printf("Ran %u layers\n", layersRun);
    printf("Total new time: %u\n", newTimeSum);
    printf("Total base time: %u\n", baseTimeSum);
    printf("Total raw time: %u\n", rawTimeSum);
    printf("Total new base: %u\n", newBaseSum);

    printf("Efficiency base/new base: %.3f (Optimal = 2)\n",
        double(baseTimeSum)/newBaseSum);
    printf("Efficiency base/ new total: %.3f (Optimal = 2)\n",
        double(baseTimeSum)/newTimeSum);
    printf("Efficiency raw/ new total: %.3f (Optimal > 2)\n",
        double(rawTimeSum)/newTimeSum);
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

#define INVALID_TRANSITION -1u

unsigned int ChainStar::getTransitionTime(const Point3& a1p1,
    const Point3& a1p2, const Point3& a2p1, const Point3& a2p2,
    const ChainLayerMeta& clm) const 
{
    return 1;
    //TODO - get transition time need a potentially massive overhaul
    if( (a1p1 == Point3::ANY || a1p2 == Point3::ANY) &&
        (a2p1 == Point3::ANY || a2p2 == Point3::ANY)
    ){
        //a special case where both a1 and a2 both have free choice
        // in theory, both or operations are x-or true
        // however, since we already treat an any position as free
        // this transition must be a completely free one
        
        //however this brings about a problem where now we 
        //  are potentially massive underestimating the time
        //  and by collapsing the minimum transitions,
        //      this transition will always be chosen
        //  so we just call this an invalid transition and return
        //      the maximum 
        // TODO - should probably be an exception
        return INVALID_TRANSITION;
    }
    if((a1p1 == Point3::ANY || a1p2 == Point3::ANY)){
        //we know that both of the a2 positions are fixed
        //and again we treat any as 0-cost
        //so we treat this as a free movement w.r.t a1
        //and only worry about the a2 time
        //again, possibly underestimating but not a big deal
        double d = getPointDistance(a2p1, a2p2);
        d /= CHAIN_MIN_SEPERATION_MM;
        return (std::ceil(d));
    }
    if((a2p1 == Point3::ANY || a2p2 == Point3::ANY)){
        //same comments as above apply here
        double d = getPointDistance(a1p1, a1p2);
        d /= CHAIN_MIN_SEPERATION_MM;
        return (std::ceil(d));
    }

    //now begin the process of actually computing the transition time

    //create the two underlying segments
    LineSegment a1Seg(a1p1, a1p2);
    LineSegment a2Seg(a2p1, a2p2);

    if(clm.canMoveSegmentPair(a1Seg, a2Seg)){
        double d = std::max(a1Seg.length(), a2Seg.length());
        d /= CHAIN_MIN_SEPERATION_MM;
        return (std::ceil(d));
    }

    //try no-oping one while the other moves
    //  then no-oping the other while the first moves
    //may overestimate the time required
    //TODO
    

    //the final operation:
    //  we know that the positions pairs are valid
    //  so there exists some transition that makes this work, somehow
    //  we can approximate this by
    //      move one agent, 1 segment out of the way
    //      move the second agent to its new position
    //      move the first agent back to its starting position (1 cost)
    //      move the first agent to its new position
    return (std::ceil(a1Seg.length()) + std::ceil(a2Seg.length()) + 2);
}

void ChainStar::doPhase1LayerRecompute(
    std::vector<PreComputeChain> &resolvedPrecomputeChainPairs,
    const ChainLayerMeta &clm)
{
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

    while (true)
    {
        if (currentPrinted.getUnsetCount() == 0)
        {
            //we have pathed the whole layer
            break;
        }

        //backfill the concurrent considerations
        while ((chainSourceIter != chainSourceEnd) &&
               (consideredChains.size() < CONCURRENT_CONSIDERATIONS_TARGET))
        {
            consideredChains.insert(*chainSourceIter);
            chainSourceIter++;
        }

        if (currentPrinted.getSetCount() == 0)
        {
            //first iteration, all chains are pending
            //do nothing
        }
        else
        {
            //cut down the pending chains for anything that overlaps
            const std::set<Chain> oldChains = std::move(consideredChains);

            for (const Chain &oldChain : oldChains)
            {
                // create the new chain(s) of portions that do not
                //  include any already-printed stuff
                // remove any chains from the set
                DynamicBitset dbsMask = clm.chainAsBitMask(oldChain);
                DynamicBitset dbsAND = (dbsMask & currentPrinted);
                if (dbsAND.getSetCount() == 0)
                {
                    //no overlap in the chain and dbsMask
                    consideredChains.insert(oldChain);
                }
                else
                {
                    //do not reinsert since no longer considered
                    //see if new remainder chains are formed
                    bool inRemainderRun = false;
                    unsigned int runLen = 0;
                    unsigned int runStart = 0;

                    unsigned int start = oldChain.getStartIndex();
                    unsigned int end = oldChain.getEndIndex();
                    if (end < start)
                    {
                        std::swap(start, end);
                    }
                    for (unsigned int i = start; i <= end; i++)
                    {
                        if (currentPrinted.at(i) == false)
                        {
                            if (inRemainderRun == false)
                            {
                                inRemainderRun = true;
                                runStart = i;
                            }
                            runLen++;
                        }
                        else
                        {
                            if (inRemainderRun)
                            {
                                Chain c(runStart, runLen, true);
                                consideredChains.insert(c);
                                consideredChains.insert(
                                    Chain(c.getEndIndex(), runLen, false));
                                inRemainderRun = false;
                                runLen = 0;
                            }
                        }
                    }
                    if (inRemainderRun)
                    {
                        Chain c(runStart, runLen, true);
                        consideredChains.insert(c);
                        consideredChains.insert(
                            Chain(c.getEndIndex(), runLen, false));
                    }
                }
            }
        }

        //TODO - move cached pairs and copy back in the ones that are still relevant
        //  This **should** provide performance improvement, but should check

        //ensure that all current chains have the collision-ness computed
        //  NOTE: chain printing concurrency is potentially not symmetric
        for (const Chain &chain1 : consideredChains)
        {
            for (const Chain &chain2 : consideredChains)
            {
                if (chain1 == chain2)
                {
                    continue;
                }

                std::pair<Chain, Chain> chainPair(chain1, chain2);
                if (cachedChainPairs.find(chainPair) == cachedChainPairs.end())
                {
                    //this is a new chain pair, do the compute
                    const unsigned int i = clm.resolveChainPair(chainPair);
                    if (i == 0)
                    {
                        //dont bother pushing the empty pairings into the set
                        continue;
                    }
                    PreComputeChain pcc(chain1, chain2, i);
                    cachedChainPairsPQ.push(pcc);
                }
            }
        }

        PreComputeChain pcc;
        bool validPCC = false;
        while (cachedChainPairsPQ.size() > 0)
        {
            pcc = cachedChainPairsPQ.top();
            cachedChainPairsPQ.pop();
            std::pair<Chain, Chain> chainPair(pcc.c1, pcc.c2);
            //erase this pair as it is no longer under consideration
            cachedChainPairs.erase(chainPair);

            //check if this is still a valid-pre cache
            //  no need to add the new segments, as
            //      the old chains loop above will take care of it
            if (consideredChains.find(pcc.c1) == consideredChains.end())
            {
                continue;
            }
            if (consideredChains.find(pcc.c2) == consideredChains.end())
            {
                continue;
            }
            if (pcc.amountPrinted == 0)
            {
                //this will ultimately terminate into a runtime error
                //  because we order on amount printed first
                continue;
            }

            validPCC = true;
            break;
        }

        if (validPCC)
        {
            const DynamicBitset dbsMask = clm.preComputeChainAsBitMask(pcc);

            //debug check
#ifdef DEBUG
            assert((currentPrinted & dbsMask).getSetCount() == 0);
#endif

            //TODO can optimize here if really wanted to
            //  dont construct new bitset & copy, just update in place
            currentPrinted = currentPrinted | dbsMask;

            reductionRounds++;

            //log info about the progression
            printf("Reduction round %u: added %u*2 segments, total %u/%u, "
                   "pending: %lu, %lu\n",
                   reductionRounds, pcc.amountPrinted, currentPrinted.getSetCount(),
                   clm.getNumPrintSegmentsInLayer(), cachedChainPairsPQ.size(),
                   consideredChains.size());

            //construct a new pcc with only printed chains
            PreComputeChain pcc_new(
                Chain(pcc.c1.getStartIndex(), pcc.amountPrinted, pcc.c1.isForward()),
                Chain(pcc.c2.getStartIndex(), pcc.amountPrinted, pcc.c2.isForward()),
                pcc.amountPrinted);

            //store this pre-compute-chain for the second phase
            resolvedPrecomputeChainPairs.push_back(pcc_new);
            continue;
        }

        if (chainSourceIter != chainSourceEnd)
        {
            //there exists more source chains we can pull from
            //  to attempt to find more pairs
            //expand five per iteration
            //  Results: this prevents the crash,
            //      but for some the chain pool grows very large
            //      this is a problem because then the n^2 effects take hold
            //  Summary: ok quick fix, but something better is needed long term
            //  TODO ^
            //  This problem generally occurs on small files
            //  that are approximately the same size as the min collision distance
            //      i.e.: bounding box is ~30mm and the collision distance is 25mm
            unsigned short ctr = 0;
            while ((chainSourceIter != chainSourceEnd) &&
                   (ctr++ < 5))
            {
                consideredChains.insert(*chainSourceIter);
                chainSourceIter++;
            }
            continue;
        }

        //all source chains considered: no pairs possible
        //  so time to start winding down and exiting

        //this is a non-valid pcc,
#ifdef DEBUG //some debug checking and printing
        printf("All dual move segments resolved.\n");
        printf("Number of printed segments: %u/%u (unprinted: %u)\n",
               currentPrinted.getSetCount(), clm.getNumPrintSegmentsInLayer(),
               currentPrinted.getUnsetCount());
        printf("Number of pending chains: %lu\n", consideredChains.size());

        unsigned int longestChain = 0;
        for (const Chain &chain1 : consideredChains)
        {
            if (chain1.getChainLength() > longestChain)
            {
                longestChain = chain1.getChainLength();
            }
        }
        printf("Longest Remaining Chain: %u\n", longestChain);

        //so lets check one thing: are there really no moves
        unsigned int longestPair = 0;
        for (const Chain &chain1 : consideredChains)
        {
            for (const Chain &chain2 : consideredChains)
            {
                unsigned int i = clm.resolveChainPair(chain1, chain2);
                if (i > longestPair)
                {
                    longestPair = i;
                }
            }
        }
        printf("Longest remaining pair: %u\n", longestPair);

        if (chainSourceIter != chainSourceEnd)
        {
            printf("WARNING! not all source chains consumed\n");
        }
        else
        {
            printf("All source chains consumed\n");
        }

        //check if the following are both true
        //  1. the remaining chains covers all the printed segments
        //  2. the remaining chains do not overlap at all;
        DynamicBitset partialDBS(currentPrinted);
        unsigned int partialSum = 0;

        for (const Chain &chain1 : consideredChains)
        {
            partialDBS = partialDBS | clm.chainAsBitMask(chain1);
            partialSum += chain1.getChainLength();
        }

        if (partialDBS.getUnsetCount() == 0)
        {
            printf("Remaining chains fully cover layer\n");
        }
        else
        {
            printf("WARNING! remaining chains do not fully cover layer.\n");
        }

        if (partialSum == (partialDBS.getSetCount() - currentPrinted.getSetCount()))
        {
            printf("Remaning chains have no overlap.\n");
        }
        else
        {
            printf("%u/%u overlap in the remaning chains set.\n",
                   partialSum, currentPrinted.getUnsetCount());
        }
#endif

        printf("%lu unmatched sub chains.\n", consideredChains.size() / 2);

        while (currentPrinted.getUnsetCount() > 0)
        {
            if (consideredChains.size() == 0)
            {
                throw std::runtime_error(
                    "Could not utilize remainder chains to cover unprinted");
            }

            //since chain is ordered in-order,
            //  this is the longest chain
            const Chain c = *(consideredChains.rbegin());
            consideredChains.erase(c);
            DynamicBitset dbs = clm.chainAsBitMask(c);

            //TODO - or sum is more efficient
            if ((dbs & currentPrinted).getSetCount() != 0)
            {
                //there was overlap in chain and printed
                continue;
            }

            //TODO - check which agent(s) can print this chain

            PreComputeChain pccNew(c, Chain::noopChain, c.getChainLength());
            currentPrinted = currentPrinted | dbs; // TODO - overload |=
            resolvedPrecomputeChainPairs.push_back(pccNew);
        }

        //since we now have full coverage:
        break; //would happen at top of loop anyway but whatever
    } // while(True)
}

LayerResults ChainStar::doRecomputeLayer(const GCodeParser& gcp, const double zLayer){

    printf("Starting layer z=%.3f\n", zLayer);

    const ChainLayerMeta clm(gcp, zLayer);

    //stores the pairs for the precompute chain that are resolved
    std::vector<PreComputeChain> resolvedPrecomputeChainPairs;

    doPhase1LayerRecompute(resolvedPrecomputeChainPairs, clm);

#ifdef DEBUG
    //debug checking to ensure this is a valid config
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
#endif //DEBUG

#ifdef DUMP_CHAINS
    std::cout << "Dumping the partial chains to a file" << std::endl;
    dumpChainPairsToFile(gcp.getFilePath(), resolvedPrecomputeChainPairs, clm, zLayer);
#endif

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
    std::vector<PositionPair> chainPositions;

    //the point representing any position (and thus a no-op move)
    //TODO - should be semi-formalized probably
    //  and not just 0,0,0 but whatever
    Point3 anyPos = Point3::ANY;

    //TODO - PROBLEM IN HERE
    //  HOW TO HANDLE THE NOOP CHAINS?
    //  THE POINTS THAT ARE ALL ZERO
    //now we need to add the endpoints to that set
    for(unsigned int i = 0; i < resolvedPrecomputeChainPairs.size(); i++){
        const PreComputeChain& pcc = resolvedPrecomputeChainPairs[i];
        auto t = PositionPair(
            ((pcc.c1.getChainLength() != 0) ? clm.getChainStartPoint(pcc.c1) : anyPos),
            ((pcc.c2.getChainLength() != 0) ? clm.getChainStartPoint(pcc.c2) : anyPos),
            i
        );

#ifdef DEBUG
        assert(std::get<0>(t).getZ() == zLayer || std::get<0>(t).getZ() == Point3::ANY.getZ());
        assert(std::get<1>(t).getZ() == zLayer || std::get<1>(t).getZ() == Point3::ANY.getZ());
#endif

        chainPositions.push_back(t);

        t = PositionPair(
            ((pcc.c1.getChainLength() != 0) ? clm.getChainEndPoint(pcc.c1) : anyPos),
            ((pcc.c2.getChainLength() != 0) ? clm.getChainEndPoint(pcc.c2) : anyPos),
            i
        );

#ifdef DEBUG
        assert(std::get<0>(t).getZ() == zLayer || std::get<0>(t).getZ() == Point3::ANY.getZ());
        assert(std::get<1>(t).getZ() == zLayer || std::get<1>(t).getZ() == Point3::ANY.getZ());
#endif

        chainPositions.push_back(t);
    }

    //stores the resolved transitions
    std::priority_queue<ResolvedTransition> transitionsPQ;

    //TODO - should re-work this to the lazy evaluation method used
    //  in the phase 1

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

            auto a1p1 = std::get<0>(chainPositions.at(i));
            auto a1p2 = std::get<1>(chainPositions.at(i));
            auto a2p1 = std::get<0>(chainPositions.at(j));
            auto a2p2 = std::get<1>(chainPositions.at(j));

            //TODO
            //calculate the transition time for this pair
            unsigned int transitionTime = getTransitionTime(
                a1p1, a1p2, a2p1, a2p2, clm
            );

            //skip these potentially invalid transitions
            //  this may cause a problem if the number of 
            //  single agent moves is larger than number of double agent moves
            if(transitionTime == INVALID_TRANSITION){
                continue;
            }

            auto t = ResolvedTransition(transitionTime, i, j);
            transitionsPQ.push(t);
        }
    }

    //bitset for tracking which positions are (not) used already
    DynamicBitset positionsBitset(chainPositions.size());

    //tracks the total transition time penalty
    unsigned int totalTransitionsTime = 0;

    //compare against two, because two points
    //  the start and end of the layer
    //  can be unset and there is no problem
    //TODO - long term
    //  can inject a layer start point to make sure that the 
    //      recomputed layer starts at some location
    //      relatively reasonable to the previous layer's end point
    while(positionsBitset.getUnsetCount() != 2){
        if(positionsBitset.getUnsetCount() < 2){
            //pretty sure this is an error and unreachable
            std::cout << "Warning: reached possibly invalid state:"
                " <2 unset in transition" << std::endl;
            break;
        }

        while(true){
            if(transitionsPQ.size() == 0){
                throw std::runtime_error("Transitions pq empty prematurely\n");
            }

            auto t = transitionsPQ.top();
            transitionsPQ.pop();

            //check if this is a new transition and is needed
            unsigned int transition_start = std::get<1>(t);
            unsigned int transition_end = std::get<2>(t);

            if(positionsBitset.at(transition_start)){
                //the transition's start point was already used
                continue;
            }

            if(positionsBitset.at(transition_end)){
                //the transition's end point was already used
                continue;
            }

            positionsBitset.set(transition_start);
            positionsBitset.set(transition_end);
            totalTransitionsTime += std::get<0>(t);
            
            break;
        }
    }

    //now compute the total path-print time
    unsigned int pathPrintTime = 0;
    for(auto pcc : resolvedPrecomputeChainPairs){
        pathPrintTime += pcc.amountPrinted;
    }

    unsigned int totalTime = pathPrintTime + totalTransitionsTime;
    unsigned int baseTime = clm.getNumPrintSegmentsInLayer();
    unsigned int rawTime = clm.getNumSegmentsInLayer();

    //debug info
    std::cout << "Total Print Time: " << pathPrintTime << std::endl;
    std::cout << "Total Transitions Time: " << totalTransitionsTime << std::endl;
    std::cout << "Total Time: " << totalTime << std::endl;

    std::cout << "Base Print Time: " << baseTime  << std::endl;
    printf("Efficiency: %.3f (Optimal = 2)\n", double(baseTime)/totalTime);
    printf("Time %%: %.3f%% (Optimal = 50%%)\n", double(totalTime*100)/(baseTime));

    std::cout << "Raw Print Time: " << rawTime << std::endl;
    printf("Efficiency: %.3f (Optimal = 2)\n", double(rawTime)/totalTime);
    printf("Time %%: %.3f%% (Optimal = 50%%)\n", double(totalTime*100)/(rawTime));

    // printf("Speedup %%: %.3f%% (Optimal = 100%%)\n", 
    //     double((baseTime-totalTime)*100)/baseTime
    // );

    //TODO - final phases thingies
    //debug check that the resulting path is
    // 1. a valid non-colliding path
    // 2. covers, with no extra segments, the original path

    //finally export to a reasonable representation

    //and return the values
    return LayerResults(totalTime, baseTime, rawTime, pathPrintTime);
}