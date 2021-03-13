#include "ChainStarHelper.h"

Chain::Chain(const unsigned int startIndex, const unsigned int chainLength, 
    const bool isForwardChain) : startIndex(startIndex), chainLength(chainLength),
    traversalDirection((isForwardChain ? 
        Chain::Direction::FORWARD : Chain::Direction::BACKWARD)
    )
{}

Chain::Chain(const Chain& other) : startIndex(other.startIndex),
    chainLength(other.chainLength), traversalDirection(other.traversalDirection)
{}

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

std::ostream& operator<<(std::ostream& os, const Chain& c){
    os << "C{";
    os << c.getStartIndex() << " " << c.getChainLength() << " ";
    os << (c.getDirection() == Chain::Direction::FORWARD ? "F" : "B");
    os << "}";

    return os;
}

const Chain Chain::noopChain(0, 0, true);