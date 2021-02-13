#pragma once

#include "pch.h"

//represents a single print chain
class Chain{
public:    
//subclasses
    enum class Direction {FORWARD, BACKWARD};

protected:
//data members

    //starting index for the chain
    unsigned int startIndex;

    //number of elements in the chain
    unsigned int chainLength;

    //direction the chain moves in the layer's provided ordering
    Direction traversalDirection;

public:
//constructors
    Chain() : startIndex(0), chainLength(0), traversalDirection(Direction::FORWARD) {};
    Chain(const unsigned int startIndex, const unsigned int chainLength, 
        const bool isForwardChain);
    Chain(const Chain& other);
    // Chain& operator=(const Chain& other);

//methods

    //get the ending index of the chain
    //  note end index is inclusive on the interval
    inline unsigned int getEndIndex(void) const {
        return ((traversalDirection == Direction::FORWARD) ?
            startIndex + chainLength - 1 : startIndex - chainLength + 1
        );
    }


//accessors

    inline unsigned int getStartIndex(void) const {return startIndex;}
    inline unsigned int getChainLength(void) const {return chainLength;}
    inline Direction getDirection(void) const {return traversalDirection;}
    inline bool isForward(void) const {return traversalDirection == Direction::FORWARD;}

    inline unsigned int at(unsigned int i) const {
        return ((traversalDirection == Direction::FORWARD) ? 
                    startIndex + i : startIndex - i
        );
    }
};

struct PreComputeChain{
public:
    Chain c1;
    Chain c2;
    unsigned int amountPrinted;

    PreComputeChain() : c1(), c2(), amountPrinted(0) {};

    PreComputeChain(const Chain& c1, const Chain& c2,
        const unsigned int amountPrinted) : c1(c1), c2(c2),
        amountPrinted(amountPrinted)
    {}

    // Manual Definitions of various move/copy assignments/constructors
    // PreComputeChain(const PreComputeChain& other) : c1(other.c1), c2(other.c2),
    //     amountPrinted(other.amountPrinted)
    // {}
    //
    // PreComputeChain& operator=(const PreComputeChain& other){
    //     c1 = other.c1;
    //     c2 = other.c2;
    //     amountPrinted = other.amountPrinted;
    // }
    //
    // PreComputeChain(PreComputeChain&& other) : c1(other.c1), c2(other.c2),
    //     amountPrinted(other.amountPrinted)
    // {}
};

bool operator<(const Chain& lhs, const Chain& rhs);
bool operator<(const PreComputeChain& lhs, const PreComputeChain& rhs);

std::ostream& operator<<(std::ostream& os, const Chain& c);