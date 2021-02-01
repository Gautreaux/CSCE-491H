#include "pch.h"

#include <set>

#include "GCodeParser.h"
#include "LayerManager.h"
#include "RecomputeState.h"
#include "UtilLib/NonReallocVector.h"

#ifdef DEBUG
#ifndef REPORTING_INTERVAL
#define REPORTING_INTERVAL 100000
#endif // REPORTING_INTERVAL
#endif // DEBUG

typedef LayerManager Default_LM_Type;
typedef RecomputeState Default_State_Type;

//State_PQ:
//  a priority queue like class
//LM_Type:
//  class for mananging the layer and providing necessary meta information
//State Representation:
//  a representation of the state

template<class State_PQ, class LM_Type=Default_LM_Type, class State_Type=Default_State_Type>
class PrunedAStarV2{
protected:
//Data Members
    
    //count of total states expanded
    unsigned int totalStatesExpanded;

    //the minimum seperation allowed in mm
    const double minSeperationMM;
    
    //the maximum number of states allowed to step back
    const unsigned int maximumStepBack;

    //the minimum efficiency allowable in a solution
    const double minimumEfficiency;
    

//Subclasses

    //compare the pointers of two State_Types base on the actual objects less than operator
    struct StatePointerCompare{
        bool operator() (const State_Type* lhs, const State_Type* rhs) const {
            return (*lhs) < (*rhs);
        }
    }; 

//Sub Typedefs
    typedef std::set<State_Type*, StatePointerCompare> State_Set_Type;

//Helper Functions


    //do a recompute on a singular layer
    void doRecomputeLayer(const GCodeParser& gcp, const double zLayer){
        //TODO

        //Object containing some meta information about the layer
        //TODO - transition layer manager into an argument?
        LM_Type lm(gcp, zLayer);

        //Object for holding yet-to-be expanded states
        State_PQ pq;

        //TODO - does the whole visited object, pointer set still make sense?

        //Object for holding already-expanded states,
        //  while a vector, will never move objects in memory once placed
        NonReallocVector<State_Type> visitedObjects;

        //Object for presence lookups within visitedObjects
        //  utilizes pointers to prevent large copies
        State_Set_Type visitedSet;

        //stores a pointer to the goal state, if found
        const State_Type* goalState = nullptr;
        
        //stores a pointer to the best state
        const State_Type* bestState = nullptr;

        //stores total new states this layer
        unsigned int statesExpandedThisLayer = 0;

        generateStartingPositions(gcp, lm, pq);

        printf("Starting recompute of z=%.03f with parameters:"
            " Min Seperation MM: %.03f, Max Stepback: %u, Minimum Efficiency: %.03f\n",
            zLayer, minSeperationMM, maximumStepBack, minimumEfficiency
        );
#ifdef DEBUG
        printf("Layer resolved %llu total starting states\n", pq.size());
#endif
        assert(pq.size() > 0);

        while(!pq.empty()){
            RecomputeState state = pq.top();

            pq.pop();

            totalStatesExpanded += 1;

            statesExpandedThisLayer += 1;

            if(isGoalState(state)){
                goalState = visitedObjects.push(state);
                break;
            }

            if(state.getEfficiency() < minimumEfficiency){
                //state to innefficient, skip state
#ifdef DEBUG
                printf("State %u, skipped (efficiency) (%.03f < %.03f)\n", 
                    statesExpandedThisLayer, state.getEfficiency(), minimumEfficiency
                );
#endif
                continue;
            }

            bool newBestState = false;
            if(bestState == nullptr){
                newBestState = true;
            }else if(bestState->getBitset().getSetCount() < state.getBitset().getSetCount()){
                //new best state
                newBestState = true;
            }else{
                //newBestState is inherently more set than the pervious best,
                //  which can cause some integer rollunders on this, which is bad
                //  so only do check on non-new-best states
                if((bestState->getBitset().getSetCount() - state.getBitset().getSetCount()) <= maximumStepBack){
                    //this state is ok on stepback, so pass
                }else{
                    //too far stepback, skip state
#ifdef DEBUG
                    printf("State %u, skipped (stepback) (set: %u, stepback: %u, best: %u)\n", 
                        statesExpandedThisLayer, state.getBitset().getSetCount(),
                        maximumStepBack, bestState->getBitset().getSetCount()
                    );
#endif
                    continue;                    
                }
            }

            //check for state presence in the visited set
            //  but to do this, we need the pointer to a constant state
            //  so that we can insert in position if not already found
            RecomputeState* statePtr = visitedObjects.push(state);
            if(newBestState){
                bestState = statePtr;
            }

            //while newBestState implies a new state, need to still add it to the set
            if(visitedSet.insert(statePtr).second){
                //this is a new state, and we can expand
                generateSuccessorStates(statePtr, gcp, lm, pq);
            }else{
                //already expanded state, so pop from the visited objects
                visitedObjects.popLast();
            }

#ifdef DEBUG
        if((statesExpandedThisLayer % REPORTING_INTERVAL) == 0){
            printf("Total %u states expanded. ", statesExpandedThisLayer);
            if(bestState != nullptr){
                printf("Pending states %llu (avg_eff: %f); Best state %u/%u printed, depth: %d, efficiency: %f\n", 
                    pq.size(), pq.getAverageEff(), bestState->getBitset().getSetCount(), lm.getTotalPrintSegments(), 
                    bestState->getDepth(), bestState->getEfficiency());
                std::cout << "\tPQ Pending Buckets: ";
                pq.printBuckets(std::cout);
                std::cout << std::endl;
            }else{
                printf("No State yet?\n");
            }
        }
#endif
        }

        if(goalState == nullptr){
#ifdef DEBUG
            printf("Layer explored %u states and did not find a goal\n", statesExpandedThisLayer);
#endif
            //TODO - this should probably raise a runtime exception
            return;
        }

        //TODO - some form of path encoding and returning

        // now we need to extract the path from the states
        std::vector<const RecomputeState*> resolvedPath;
        while(goalState != nullptr){
            resolvedPath.push_back(goalState);
            goalState = goalState->getParent();
        }

        std::cout << "Resolved path:" << std::endl;
        for(auto i = resolvedPath.rbegin(); i != resolvedPath.rend(); i++){
            printVerbose(std::cout, **i, gcp, lm);
        }
    }

//Operations Functions

    //return true iff this is a goal state
    bool isGoalState(const State_Type& state) const {return (state.getBitset().getUnsetCount() == 0);}

    //push all the starting positions for the layer into the Priority queue
    virtual void generateStartingPositions(const GCodeParser& gcp, const LM_Type& lm, State_PQ& pq) = 0;

    //push all successor states into the Priority Queue
    virtual void generateSuccessorStates(const State_Type* state, const GCodeParser& gcp, 
        const LM_Type& lm, State_PQ& pq) = 0;

    //return true if the positions are valid
    virtual bool isValidPositionPair(const Point3& p1, const Point3& p2) const = 0;

    //return true if the segment are valid
    virtual bool isValidSegmentsPair(const GCodeSegment& s1, const GCodeSegment& s2) const = 0;

    //return true if the segment can be printed while the agent no-ops
    virtual bool isValidSegmentNOOP(const GCodeSegment& a1Segment, const Point3& a2Pos) const = 0;
    virtual bool isValidSegmentNOOP(const Point3& a1Pos, const GCodeSegment& a2Segment) const = 0;

    void printVerbose(std::ostream& os, const RecomputeState& state, const GCodeParser& gcp, const LayerManager& lm) const {
        os << "A1: " << lm.getPoint3FromPos(state.getA1PosIndex()) << "(" << state.getA1PosIndex() << "), ";
        os << "A2: " << lm.getPoint3FromPos(state.getA2PosIndex()) << "(" << state.getA2PosIndex() << "), ";
        os << "Depth: " << state.getDepth() << ", Efficiency: " << state.getEfficiency() << ", ";
        os << "BitData: ";
        state.getBitset().printBitData(os);
        os << std::endl;
    }

public:
//Constructors, Destructors
    PrunedAStarV2(const GCodeParser& gcp, const double minSeperationMM = 25,
        const unsigned int maximumStepBack = (unsigned int)(-1), const double minimumEfficiency = 0.0) :
        totalStatesExpanded(0), minSeperationMM(minSeperationMM),
        maximumStepBack(maximumStepBack), minimumEfficiency(minimumEfficiency)
    {
    }
    ~PrunedAStarV2(){}

    // do a recompute layer by layer for the parser
    void doRecompute(const GCodeParser& gcp){
        for(auto layer = gcp.layers_begin(); layer < gcp.layers_end(); layer++){
            doRecomputeLayer(gcp, *layer);
#ifdef SINGLE_LAYER_ONLY
            break;
#endif
        }
    }


//Getters
    inline const unsigned int getTotalStatesExpanded(void) const {return totalStatesExpanded;}
};
