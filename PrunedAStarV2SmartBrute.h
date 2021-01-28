#pragma once

#include "pch.h"

#include <vector>

#include "PrunedAStarV2BruteForce.h"


class PrunedAStarV2SmartBrute : public PrunedAStarV2BruteForce {
protected:
    virtual void generateStartingPositions(const GCodeParser& gcp, const Default_LM_Type& lm, Brute_Force_PQ_Type& pq){
        //Basic IDEA: create only the positions that are endpoints of the chain

        DynamicBitset startBitset(lm.getTotalPrintSegments());
        auto chainStartEndPoints = lm.getChainStartEndIndexes();
        
        //print the position index and the positions for all chain start/stop
        std::cout << "Total Chains: " << lm.getTotalChains() << std::endl; 
        // for(auto e : chainStartEndPoints){
        //     std::cout << e << " " << lm.getPoint3FromPos(e) << std::endl;
        // }
            
        for(const Position_Index i : chainStartEndPoints){

            const Point3& pi = lm.getPoint3FromPos(i);

            for(const Position_Index j : chainStartEndPoints){
                if(i == j){
                    continue;
                }

                const Point3& pj = lm.getPoint3FromPos(j);

                if(isValidPositionPair(pi, pj)){
                    pq.push(Default_State_Type(i,j,0, startBitset, nullptr));
                }
            }
        }
    }

    // virtual void generateSuccessorStates(const Default_State_Type* state, const GCodeParser& gcp,
    //     const Default_LM_Type& lm, Brute_Force_PQ_Type& pq)
    // {
    //     //TODO
    // }

public:
    PrunedAStarV2SmartBrute(const GCodeParser& gcp, const double MinSeperationMM,
        const unsigned int maximumStepBack, const double minimumEfficiency) :
        PrunedAStarV2BruteForce(gcp, MinSeperationMM, maximumStepBack, minimumEfficiency)
    {}
};