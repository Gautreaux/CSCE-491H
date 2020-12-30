#include "DynamicBitset.h"

DynamicBitset::DynamicBitset(void) :
    numBits(0), setCount(0), bitset(nullptr)
{
}

DynamicBitset::DynamicBitset(unsigned int size){
    numBits = size;
    setCount = 0;
    bitset = new char[DBS_NUM_BYTES];
    //printf("DBS Alloc: %p @ %p\n", bitset, this);
    assert(bitset != nullptr);
    memset(bitset, 0, DBS_NUM_BYTES);
}

//copy constructor
DynamicBitset::DynamicBitset(const DynamicBitset& other){
    numBits = other.numBits;
    setCount = other.setCount;
    bitset = new char[DBS_NUM_BYTES];
    //printf("DBS Alloc: %p @ %p\n", bitset, this);
    assert(bitset != nullptr);
    memcpy(bitset, other.bitset, DBS_NUM_BYTES);
}

DynamicBitset::~DynamicBitset(void){
    if(bitset != nullptr){
        //printf("DBS DeAlloc: %p @ %p\n", bitset, this);
        delete[] bitset;
        bitset = nullptr;
    }
}

// copy assignment
DynamicBitset& DynamicBitset::operator=(const DynamicBitset& other){
    if(bitset != nullptr){
        delete[] bitset;
        bitset = nullptr;
    }

    numBits = other.numBits;
    setCount = other.setCount;
    //printf("DBS Alloc: %p @ %p\n", bitset, this);
    assert(bitset != nullptr);
    memcpy(bitset, other.bitset, DBS_NUM_BYTES);
}

//move assignment
DynamicBitset& DynamicBitset::operator=(DynamicBitset&& other){
    if(bitset != nullptr){
        delete[] bitset;
        bitset = nullptr;
    }

    numBits = other.numBits;
    setCount = other.setCount;
    bitset = other.bitset;

    other.bitset = nullptr;
    other.numBits = 0;
    other.setCount = 0;
}

bool DynamicBitset::at(const unsigned int i) const {
    if(i < 0 || i > numBits){
        throw std::out_of_range("");
    }

    int byte = i / DBS_SOC;
    int remainder = i % DBS_SOC;
    return (((char*)bitset)[byte] & (1 << remainder) != 0);
}

bool DynamicBitset::set(const unsigned int i, const bool value){
    if(i < 0 || i > numBits){
        throw std::out_of_range("");
    }

    int byte = i / DBS_SOC;
    int remainder = i % DBS_SOC;
    bool toReturn = (((char*)bitset)[byte] & (1 << remainder) != 0);
    
    if(value == true){
        ((char*)bitset)[byte] |= (1 << remainder);
    }else{
        ((char*)bitset)[byte] &= ((1 << DBS_SOC) ^ (1 << remainder));
    }

    //update the setcount if the bit actually changed state
    if (value == false && toReturn == true){
        setCount--;
    }else if(value == true && toReturn == false){
        setCount++;
    }

    return toReturn;
}