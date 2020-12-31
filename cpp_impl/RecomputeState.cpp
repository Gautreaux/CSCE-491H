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
    if(lhs.getA1PosIndex() < rhs.getA1PosIndex()){
        return true;
    }
    if(lhs.getA1PosIndex() > rhs.getA1PosIndex()){
        return false;
    }
    //A1s Match

    if(lhs.getA2PosIndex() < rhs.getA2PosIndex()){
        return true;
    }
    if(lhs.getA2PosIndex() > rhs.getA2PosIndex()){
        return false;
    }
    //A1 and A2 Match

    //dont care about stepDepth

    return lhs.getBitset() < rhs.getBitset();
}

bool operator==(const RecomputeState& lhs, const RecomputeState& rhs){
    //allow for the position indicies to be transposed (for now)
    if(!(lhs.getA1PosIndex() == rhs.getA1PosIndex() || lhs.getA1PosIndex() == rhs.getA2PosIndex())){
        return false;
    }

    if(!(lhs.getA2PosIndex() == rhs.getA1PosIndex() || lhs.getA2PosIndex() == rhs.getA2PosIndex())){
        return false;
    }

    //dont really care about the stepDepth ATM

    return lhs.getBitset() == rhs.getBitset();
}