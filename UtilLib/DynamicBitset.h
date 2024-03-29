#pragma once

#include "../pch.h"

#define DBS_SOC 8
#define DBS_NUM_BYTES ((numBits % DBS_SOC) ? numBits / DBS_SOC + 1 : numBits / DBS_SOC)

class DynamicBitset{
private:
    unsigned int numBits;
    unsigned int setCount; // number of bits set
    char* bitset;
public:
    DynamicBitset(void);
    DynamicBitset(const unsigned int size);
    DynamicBitset(const DynamicBitset& other);
    ~DynamicBitset(void);

    DynamicBitset& operator=(const DynamicBitset& other);
    DynamicBitset& operator=(DynamicBitset&& other);

    //get the value of the bit at index i
    bool at(const unsigned int i) const;

    //set the value of the bit at index i 
    //  returns the state before the bit was set
    bool set(const unsigned int i, const bool value=true);

    unsigned int inline size(void) const {return numBits;}

    bool inline isFull(void) const {return setCount == numBits;}
    bool inline isEmpty(void) const {return setCount == 0;}

    // return the number of bits that are set
    unsigned int inline getSetCount(void) const {return setCount;}

    // return the number of bits that are not set
    unsigned int inline getUnsetCount(void) const {return numBits - setCount;}

    //The memory size of this object
    // for something i'm planning later
    unsigned int inline memorySize(void) const {return sizeof(DynamicBitset) + DBS_NUM_BYTES;}

    //compare the actual buffers
    short inline compareBits(const DynamicBitset& other) const {return memcmp(bitset, other.bitset, DBS_NUM_BYTES);}

    //print the bits of this object in hexadecimal format into the stream
    void printBitData(std::ostream& os) const;
};

bool operator<(const DynamicBitset& lhs, const DynamicBitset& rhs);
bool operator==(const DynamicBitset& lhs, const DynamicBitset& rhs);

//bitwise and of two bitsets up to the length of the lesser of the two
DynamicBitset operator&(const DynamicBitset& lhs, const DynamicBitset& rhs);

//bitwise and of two bitsets up to the length of the lesser of the two
DynamicBitset operator|(const DynamicBitset& lhs, const DynamicBitset& rhs);