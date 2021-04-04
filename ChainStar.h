#pragma once

#include "pch.h"

#include <algorithm>
#include <ctime>
#include <functional>
#include <set>
#include <tuple>
#include <vector>
#include <queue>

#include "ChainStarHelper.h"
#include "ChainStarLog.h"
#include "ChainLayerMeta.h"
#include "UtilLib/DynamicBitset.h"
#include "GCodeLib/GCodeParser.h"

#include "RecomputeFrameworks.h"


// #define SINGLE_LAYER_ONLY
// #warning "Forcing single layer mode"

//used to prevent wasting time on a bunch of small chains
#ifndef CONCURRENT_CONSIDERATIONS_TARGET
#define CONCURRENT_CONSIDERATIONS_TARGET 100
#endif

#define INVALID_TRANSITION -1u

//tuple ordering
//  chain 1 position
//  chain 2 position
//  PCC_Pair index (because you cant transition from self to self)
typedef std::tuple<Point3, Point3, unsigned int> PositionPair;

//tuple ordering
//  transition cost
//  PCC_pair transition start index
//  PCC_pair transition end index
typedef std::tuple<unsigned int, unsigned int, unsigned int> ResolvedTransition;

//tuple ordering
//  transition cost
//  state a index
//  state b index
typedef std::tuple<unsigned int, unsigned int, unsigned int>
    StateTransition;

//tuple ordering
//  newTime - new time cost for layer
//  baseTime - original print time cost for layer
//  rawTime - original total time for layer
//  newBase - new print time cost for layer
typedef std::tuple<unsigned int, unsigned int, unsigned int, unsigned int>
    LayerResults;

//manages the recomputing of a file
//  T should extend ChainLayerMeta
template <class CLM_Type>
class ChainStar{

protected:
    struct State{
    public:
        Point3 a1Pos;
        Point3 a2Pos;
        unsigned int partnerIndex;
        unsigned int chainGroup;
        bool hasBeenUsed;
    };

    //run recompute on the specific layer
    LayerResults doRecomputeLayer(const GCodeParser& gcp, const double zLayer);

    //do the phase 1 layer recompute:
    //  Matching segment chains together into pairs
    //      using CLM functions to determine if the chains can be concurrent
    //      specifically, the resolveChainPairs function
    void doPhase1LayerRecompute(
        std::vector<PreComputeChain>& resolvedPrecomputeChainPairs,
        const CLM_Type& clm
    );

    //do the phase 2 layer recompute:
    //  Linking endpoints of chain pairs
    unsigned int doPhase2LayerRecompute(
        std::vector<PreComputeChain>& resolvedPrecomputeChainPairs,
        const CLM_Type& clm
    );

public:

//constructors
    ChainStar(){};

//methods
    //recompute on the provided GCP file
    //  define pre-processor flag 'SINGLE_LAYER_ONLY' to run just the first layer
    //TODO - return type?
    void doRecompute(const GCodeParser& gcp);
};

template <class CLM_Type> 
void ChainStar<CLM_Type>::doRecompute(const GCodeParser& gcp){
    assert(gcp.isValid());

    unsigned int totalTimeSum = 0;
    unsigned int rawTimeSum = 0;
    unsigned int rawPrintTimeSum = 0;
    unsigned int printTimeSum = 0;
    unsigned int layersRun = 0;

    clock_t startTime = clock();

    for(const double layer : gcp.getLayerVecRef()){
        //TODO - some return type stuff here?
        LayerResults r = doRecomputeLayer(gcp, layer);

        totalTimeSum += std::get<0>(r);
        rawTimeSum += std::get<1>(r);
        rawPrintTimeSum += std::get<2>(r);
        printTimeSum += std::get<3>(r);

        layersRun++;
#ifdef SINGLE_LAYER_ONLY
        break;
#endif
    }

    clock_t endTime = clock();
    printf("Recompute duration (s): %.3f\n", double(endTime-startTime)/CLOCKS_PER_SEC);

    printf("Ran %u layers\n", layersRun);
    printf("Total new time: %u\n", totalTimeSum);
    printf("Total base time: %u\n", rawTimeSum);
    printf("Total raw print time: %u\n", rawPrintTimeSum);
    printf("Total new print time: %u\n", printTimeSum);

    printf("Total Efficiency: base/new base: %.3f (Optimal > 2)\n",
        double(rawTimeSum)/totalTimeSum);
    printf("Print Efficiency: print/ new print: %.3f (Optimal = 2)\n",
        double(rawPrintTimeSum)/printTimeSum);
}

template <class CLM_Type>
LayerResults ChainStar<CLM_Type>::doRecomputeLayer(
    const GCodeParser &gcp, const double zLayer)
{

    printf("Starting layer z=%.3f\n", zLayer);

    CLM_Type clm(gcp, zLayer);

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

    //time spent printing segments in the recomputed file
    unsigned int printTime = 0;
    for(auto& c : resolvedPrecomputeChainPairs){
        printTime += c.amountPrinted;
    }

    std::cout << "Phase 1 completed." << std::endl;

    //time spend transitioning in the recomputed file
    unsigned int totalTransitionsTime = doPhase2LayerRecompute(
        resolvedPrecomputeChainPairs, clm);

    //total time in the recomputed file
    unsigned int totalTime = printTime + totalTransitionsTime;

    //total time in the original file
    unsigned int rawTime = clm.getNumSegmentsInLayer();

    //time spend printing in the original file
    unsigned int rawPrintTime = clm.getNumPrintSegmentsInLayer();

    //debug info
    std::cout << "Total Print Time: " << printTime << std::endl;
    std::cout << "Total Transitions Time: " << totalTransitionsTime << std::endl;
    std::cout << "Total Time: " << totalTime << std::endl;

    std::cout << "Base Print Time: " << rawTime  << std::endl;
    printf("Efficiency: %.3f (Optimal = 2)\n", double(rawTime)/totalTime);
    printf("Time %%: %.3f%% (Optimal = 50%%)\n", double(totalTime*100)/(rawTime));

    std::cout << "Raw Print Time: " << rawPrintTime << std::endl;
    printf("Efficiency: %.3f (Optimal = 2)\n", double(rawPrintTime)/totalTime);
    printf("Time %%: %.3f%% (Optimal = 50%%)\n", double(totalTime*100)/(rawPrintTime));

    //TODO - final phases thingies
    //debug check that the resulting path is
    // 1. a valid non-colliding path
    // 2. covers, with no extra segments, the original path

    //finally export to a reasonable representation

    //and return the values
    return LayerResults(totalTime, rawTime, rawPrintTime, printTime);
}

template <class CLM_Type>
void ChainStar<CLM_Type>::doPhase1LayerRecompute(
    std::vector<PreComputeChain> &resolvedPrecomputeChainPairs,
    const CLM_Type &clm)
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
            // printf("Reduction round %u: added %u*2 segments, total %u/%u, "
            //        "pending: %lu, %lu\n",
            //        reductionRounds, pcc.amountPrinted, currentPrinted.getSetCount(),
            //        clm.getNumPrintSegmentsInLayer(), cachedChainPairsPQ.size(),
            //        consideredChains.size());
            if(reductionRounds % 100 == 0){
                std::cout << "Completed Reduction round " << reductionRounds << " printed " << currentPrinted.getSetCount() << "/" << clm.getNumPrintSegmentsInLayer() << std::endl;
            }

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

template <class CLM_Type>
unsigned int ChainStar<CLM_Type>::doPhase2LayerRecompute(
    std::vector<PreComputeChain>& resolvedPrecomputeChainPairs,
    const CLM_Type& clm)
{
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

    std::vector<ChainStar::State> stateList;
    unsigned int numChainPairs = 0;

    for(auto& chainPair : resolvedPrecomputeChainPairs){

        State s1,s2;
        s1.a1Pos = ((chainPair.c1.isNoopChain()) ? Point3::ANY : clm.getChainStartPoint(chainPair.c1));
        s2.a1Pos = ((chainPair.c1.isNoopChain()) ? Point3::ANY : clm.getChainEndPoint(chainPair.c1));
        s1.a2Pos = ((chainPair.c2.isNoopChain()) ? Point3::ANY : clm.getChainStartPoint(chainPair.c2));
        s2.a2Pos = ((chainPair.c2.isNoopChain()) ? Point3::ANY : clm.getChainEndPoint(chainPair.c2));

        s1.hasBeenUsed = false;
        s2.hasBeenUsed = false;

        s1.partnerIndex = stateList.size() + 1;
        s2.partnerIndex = s1.partnerIndex - 1;

        s1.chainGroup = numChainPairs;
        s2.chainGroup = numChainPairs;

        stateList.push_back(s1);
        stateList.push_back(s2);

        numChainPairs++;
    }

#ifdef DEBUG
    std::cout << "Total chain pairs: " << numChainPairs << std::endl;
    std::cout << "Number states: " << stateList.size() << std::endl;
    assert(stateList.size() == (numChainPairs * 2));
#endif //DEBUG

    std::priority_queue<StateTransition> pq;

    for(unsigned int i = 0; i < stateList.size(); i++){
        for(unsigned int j = i+1; j < stateList.size(); j++){
            if(stateList.at(i).partnerIndex == j){
                continue;
            }

            auto t = clm.getTransitionTime(
                stateList.at(i).a1Pos, stateList.at(j).a1Pos,
                stateList.at(i).a2Pos, stateList.at(j).a2Pos
            );
            pq.emplace(-t, i, j);
        }
    }

#ifdef DEBUG
    std::cout << "Precompute done, now linking. Precompute QTY: " << pq.size() << std::endl;
#endif //DEBUG

    unsigned int statesUsed = 0;
    unsigned int totalTimeTransition = 0;
    while(statesUsed < (stateList.size() - 2))
    {
        if(pq.empty()){
            throw std::runtime_error("Presumably invalid state reached?");
        }

        auto& t = pq.top();
        const auto transitionTime = -std::get<0>(t);
        State& state1 = stateList.at(std::get<1>(t));
        State& state2 = stateList.at(std::get<2>(t));
        pq.pop();

        if(state1.hasBeenUsed){
            continue;
        }
        if(state2.hasBeenUsed){
            continue;
        }
        if(state1.chainGroup == state2.chainGroup){
            continue;
        }

        statesUsed += 2;
        totalTimeTransition += transitionTime;

        state1.hasBeenUsed = true;
        state2.hasBeenUsed = true;

        State& state1OldPartner = stateList.at(state1.partnerIndex);
        State& state2OldPartner = stateList.at(state2.partnerIndex);

#ifdef DEBUG
        assert(state1OldPartner.hasBeenUsed == false);
        assert(state2OldPartner.hasBeenUsed == false);

        assert(state1OldPartner.chainGroup == state1.chainGroup);
        assert(state2OldPartner.chainGroup == state2.chainGroup);
#endif //DEBUG

        state2.chainGroup = state1.chainGroup;
        state2OldPartner.chainGroup = state1.chainGroup;

        state1OldPartner.partnerIndex = state2.partnerIndex;
        state2OldPartner.partnerIndex = state1.partnerIndex;
    }

#ifdef DEBUG
    std::cout << "Linking finished, doing debugging checks.";
    std::vector<unsigned int> unusedStateIndexes;
    for(unsigned int i = 0; i < stateList.size(); i++){
        if(stateList.at(i).hasBeenUsed == false){
            unusedStateIndexes.push_back(i);
        }
    }

    assert(unusedStateIndexes.size() == 2);

    std::cout << "Unused state indexes: " << unusedStateIndexes[0] << " " << unusedStateIndexes[1] << std::endl;

    State& s1 = stateList[unusedStateIndexes[0]];
    State& s2 = stateList[unusedStateIndexes[1]];

    assert(s1.partnerIndex == unusedStateIndexes[1]);
    assert(s2.partnerIndex == unusedStateIndexes[0]);
#endif //DEBUG

    return totalTimeTransition;
}