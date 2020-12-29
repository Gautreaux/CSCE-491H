#include "DynamicBitset.h"

DynamicBitset::DynamicBitset(unsigned int size){
    numBits = size;
    setCount = 0;
    bitset = malloc(DBS_NUM_BYTES);
    assert(bitset != nullptr);
    memset(bitset, 0, DBS_NUM_BYTES);
}

DynamicBitset::~DynamicBitset(void){
    if(bitset != nullptr){
        free(bitset);
    }
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