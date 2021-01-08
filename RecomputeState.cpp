# include "RecomputeState.h"

RecomputeState::RecomputeState(void) : 
        agent1PositionIndex(0),
        agent2PositionIndex(0),
        stepDepth(0),
        bitset(),
        parentState(nullptr)
{}

RecomputeState::RecomputeState(unsigned int a1PosInd, unsigned int a2PosInd,
        unsigned int sd, const DynamicBitset& dbs, const RecomputeState* parent) :
    agent1PositionIndex(a1PosInd), agent2PositionIndex(a2PosInd),
    stepDepth(sd), bitset(dbs), 
    parentState(parent)
{
#ifdef DEBUG_4
    std::cout <<"Constructed new recompute state " << *this << std::endl;
#endif
}

//copy constructor
RecomputeState::RecomputeState(const RecomputeState& other) :
        agent1PositionIndex(other.agent1PositionIndex),
        agent2PositionIndex(other.agent2PositionIndex),
        stepDepth(other.stepDepth),
        bitset(other.bitset),
        parentState(other.parentState)
{}


//move constructor
//  TODO - not a true move because bitset is copy-constructed (i think)
RecomputeState::RecomputeState(RecomputeState&& other) : 
        agent1PositionIndex(other.agent1PositionIndex),
        agent2PositionIndex(other.agent2PositionIndex),
        stepDepth(other.stepDepth),
        bitset(other.bitset),
        parentState(other.parentState)
{
    other.agent1PositionIndex = 0;
    other.agent2PositionIndex = 0;
    other.stepDepth = 0;
    other.parentState = nullptr;
}

//copy assignment
RecomputeState& RecomputeState::operator=(const RecomputeState& other){
    if(this == &other){
        return *this;
    }

    agent1PositionIndex = other.agent1PositionIndex;
    agent2PositionIndex = other.agent2PositionIndex;
    stepDepth = other.stepDepth;
    bitset = other.bitset;
    parentState = other.parentState;
    return *this;
}

//move assignment
RecomputeState& RecomputeState::operator=(RecomputeState&& other){
    if(this == &other){
        return *this;
    }

    agent1PositionIndex = other.agent1PositionIndex;
    agent2PositionIndex = other.agent2PositionIndex;
    stepDepth = other.stepDepth;
    bitset = std::move(other.bitset);
    parentState = other.parentState;

    other.agent1PositionIndex = 0;
    other.agent2PositionIndex = 0;
    other.stepDepth = 0;
    other.parentState = nullptr;

    return *this;
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

    // depth matching shouldnt? matter
    if(lhs.getDepth() < rhs.getDepth()){
        return true;
    }
    if(lhs.getDepth() > rhs.getDepth()){
        return false;
    }
    // //a1, a2, and depth all match

    return lhs.getBitset() < rhs.getBitset();
}

bool operator==(const RecomputeState& lhs, const RecomputeState& rhs){
    if(lhs.getA1PosIndex() != rhs.getA1PosIndex()){
        return false;
    }
    if(lhs.getA2PosIndex() != rhs.getA2PosIndex()){
        return false;
    }
    if(lhs.getDepth() != rhs.getDepth()){
        return false;
    }

    // //allow for the position indicies to be transposed (for now)
    // if(!(lhs.getA1PosIndex() == rhs.getA1PosIndex() || lhs.getA1PosIndex() == rhs.getA2PosIndex())){
    //     return false;
    // }

    // if(!(lhs.getA2PosIndex() == rhs.getA1PosIndex() || lhs.getA2PosIndex() == rhs.getA2PosIndex())){
    //     return false;
    // }

    //dont really care about the stepDepth ATM

    return lhs.getBitset() == rhs.getBitset();
}


std::ostream &operator<<(std::ostream& os, const RecomputeState& rs){
    os << "A1: " << rs.getA1PosIndex() << ", A2: " << rs.getA2PosIndex() << ", ";
    os << "Depth: " << rs.getDepth() << ", Unset: " << rs.getBitset().getUnsetCount();
#ifdef DEBUG_1
    os << ", BitData: ";
    rs.getBitset().printBitData(os);
#endif
    return os;
}