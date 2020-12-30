# include "RecomputeState.h"

RecomputeState::RecomputeState(void) : 
        agent1PositionIndex(0),
        agent2PositionIndex(0),
        stepDepth((1 << sizeof(stepDepth) - 1)),
        bitset()
{}

RecomputeState::RecomputeState(unsigned int a1PosInd, unsigned int a2PosInd,
        unsigned int sd, const DynamicBitset& dbs) :
    agent1PositionIndex(a1PosInd), agent2PositionIndex(a2PosInd),
    stepDepth(sd), bitset(dbs)
{
}

//copy constructor
RecomputeState::RecomputeState(const RecomputeState& other) :
        agent1PositionIndex(other.agent1PositionIndex),
        agent2PositionIndex(other.agent2PositionIndex),
        stepDepth(other.stepDepth),
        bitset(other.bitset)
{}


//move constructor
//  TODO - not a true move because bitset is copy-constructed (i think)
RecomputeState::RecomputeState(RecomputeState&& other) : 
        agent1PositionIndex(other.agent1PositionIndex),
        agent2PositionIndex(other.agent2PositionIndex),
        stepDepth(other.stepDepth),
        bitset(other.bitset)
{
    other.agent1PositionIndex = 0;
    other.agent2PositionIndex = 0;
    other.stepDepth = (1 << sizeof(unsigned int) - 1);
}

//copy assignment
RecomputeState& RecomputeState::operator=(const RecomputeState& other){
    agent1PositionIndex = other.agent1PositionIndex;
    agent2PositionIndex = other.agent2PositionIndex;
    stepDepth = other.stepDepth;
    bitset = other.bitset;
}

//move assignment
RecomputeState& RecomputeState::operator=(RecomputeState&& other){
    agent1PositionIndex = other.agent1PositionIndex;
    agent2PositionIndex = other.agent2PositionIndex;
    stepDepth = other.stepDepth;
    bitset = std::move(other.bitset);
}

unsigned int RecomputeState::distToGoalEst(void) const {
    return bitset.getUnsetCount() / 2;
}

unsigned int RecomputeState::totalPathLenEst(void) const {
    return stepDepth + distToGoalEst();
}

bool operator<(const RecomputeState& lhs, const RecomputeState& rhs){
    return &lhs < &rhs;
}