#pragma once

#include "pch.h"

#include "PQWrapper.h"
#include "PrunedAStarV2.h"

//compare based on greatest depth (or is this least depth)
//  greatest depth expanded first
struct DepthCompare {
public:
    bool operator()(const Default_State_Type& lhs, const Default_State_Type& rhs){
        if(lhs.getDepth() < rhs.getDepth()){
            return true;
        }else if(lhs.getDepth() > rhs.getDepth()){
            return false;
        }
        return lhs < rhs;
    }
};

//expand most efficient state first
struct EfficiencyCompare{
public:
    bool operator()(const Default_State_Type& lhs, const Default_State_Type& rhs){
        if(lhs.getEfficiency() < rhs.getEfficiency()){
            return true;
        }else if(lhs.getEfficiency() > rhs.getEfficiency()){
            return false;
        }
        return lhs < rhs;
    }
};

// typedef PQWrapper<DepthCompare> Brute_Force_PQ_Type;
typedef PQWrapper<EfficiencyCompare> Brute_Force_PQ_Type;

class PrunedAStarV2BruteForce : public PrunedAStarV2<Brute_Force_PQ_Type> {
protected:
    virtual void generateStartingPositions(const GCodeParser& gcp, const Default_LM_Type& lm, Brute_Force_PQ_Type& pq){
        // unsigned int totalPositions = lm.getTotalPositions();s
        DynamicBitset startBitset(lm.getTotalPrintSegments());

        for(Position_Index i = 0; i < lm.getTotalPositions(); i++){

            const Point3& pi = lm.getPoint3FromPos(i);

            for(Position_Index j = 0; j < lm.getTotalPositions(); j++){
                if(i == j){
                    continue;
                }

                const Point3& pj = lm.getPoint3FromPos(j);

                if(isValidPositionPair(pi, pj)){
                    pq.push(Default_State_Type(i,j,0,startBitset, nullptr));
                }
            }
        }
    }

    virtual void generateSuccessorStates(const Default_State_Type* state, const GCodeParser& gcp,
        const Default_LM_Type& lm, Brute_Force_PQ_Type& pq);

    virtual inline bool isValidPositionPair(const Point3& p1, const Point3& p2) const {
        return getPointDistance(p1, p2) >= minSeperationMM;
    }

    virtual inline bool isValidSegmentsPair(const GCodeSegment& s1, const GCodeSegment& s2) const {
        return s1.minSeperationDistance(s2) >= minSeperationMM;
    }

    virtual inline bool isValidSegmentNOOP(const GCodeSegment& a1Segment, const Point3& a2Pos) const {
        return a1Segment.minSeperationDistance(a2Pos) >= minSeperationMM;
    } 

    virtual inline bool isValidSegmentNOOP(const Point3& a1Pos, const GCodeSegment& a2Segment) const {
        return a2Segment.minSeperationDistance(a1Pos) >= minSeperationMM;
    }
    
public:
    PrunedAStarV2BruteForce(const GCodeParser& gcp, const double MinSeperationMM,
        const unsigned int maximumStepBack, const double minSeperationMM) :
        PrunedAStarV2(gcp, MinSeperationMM, maximumStepBack, minSeperationMM)
    {}

};

