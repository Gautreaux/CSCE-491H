#include "DynamicBitset.h"

DynamicBitset::DynamicBitset(void) :
    numBits(0), setCount(0), bitset(nullptr)
{
#ifdef DEBUG_4
    std::cout << "DBS void constructor: ";
    printBitData(std::cout);
    std::cout << std::endl;

#endif
}

DynamicBitset::DynamicBitset(unsigned int size){
    numBits = size;
    setCount = 0;
    bitset = new char[DBS_NUM_BYTES];
    //printf("DBS Alloc: %p @ %p\n", bitset, this);
    assert(bitset != nullptr);
    memset(bitset, 0, DBS_NUM_BYTES);

#ifdef DEBUG_4
    std::cout << "DBS size constructor: ";
    printBitData(std::cout);
    std::cout << std::endl;
#endif
}

//copy constructor
DynamicBitset::DynamicBitset(const DynamicBitset& other){
    numBits = other.numBits;
    setCount = other.setCount;
    bitset = new char[DBS_NUM_BYTES];
    //printf("DBS Alloc: %p @ %p\n", bitset, this);
    assert(bitset != nullptr);
    memcpy(bitset, other.bitset, DBS_NUM_BYTES);

#ifdef DEBUG_4
    std::cout << "DBS copy constructor: ";
    printBitData(std::cout);
    std::cout << std::endl;
#endif
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
    bitset = new char[DBS_NUM_BYTES];
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
#ifdef DEBUG_4
    std::cout << "Checking bitset for pos " << i << std::endl << "Current bit data: ";
    printBitData(std::cout);
    std::cout << std::endl;
#endif
    if(i < 0 || i >= numBits){
        throw std::out_of_range("");
    }

    int byte = i / DBS_SOC;
    int remainder = i % DBS_SOC;

#ifdef DEBUG_4
    std::cout << "DBS_SOC = " << DBS_SOC << ", DBS_NUM_BYTES = " << DBS_NUM_BYTES << ", numBits = " << numBits << ", byte = " << byte << ", remainder = " << remainder << std::endl;
    std::cout << "\t bitset[byte] = ";
    char b = bitset[byte];
    std::cout << int(b) << " = ";
    for(int k = 7; k >= 0; k--){
        char c = (b & (1 << k)) != 0 ? '1' : '0';
        std::cout << c;
    }
    std::cout << std::endl;

    std::cout << "Current bit data: ";
    printBitData(std::cout);
    std::cout << std::endl;

    std::cout << "Final Result: " << (bitset[byte] & (1 << remainder) != 0) << std::endl;
#endif

    return (bitset[byte] & (1 << remainder) != 0);
}

bool DynamicBitset::set(const unsigned int i, const bool value){
#ifdef DEBUG_4
    std::cout << "DBS setting bit to value: " << i << " " << value << std::endl;
#endif
    if(i < 0 || i > numBits){
        throw std::out_of_range("");
    }

    int byte = i / DBS_SOC;
    int remainder = i % DBS_SOC;
    bool toReturn = (bitset[byte] & (1 << remainder) != 0);
    
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

void DynamicBitset::printBitData(std::ostream& os) const {
    char lookup[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    for(int i = 0; i < DBS_NUM_BYTES; i++){
        // char lsb = ((1 << 4) - 1) & bitset[i];
        // char msb = (((1 << 8) - 1) - ((1 << 4) - 1) & bitset[i]) >> 4;
        // std:: cout << "(" << int(msb) << "," << int(lsb) << ")";
        // os << lookup[msb];
        // os << lookup[lsb];
        std::cout << (int)bitset[i] << " ";
    }
}

bool operator<(const DynamicBitset& lhs, const DynamicBitset& rhs){
    if(lhs.size() < rhs.size()){
        return true;
    }
    if(lhs.size() > rhs.size()){
        return false;
    }
    //Size matches

    if(lhs.getSetCount() < rhs.getSetCount()){
        return true;
    }
    if(lhs.getSetCount() > rhs.getSetCount()){
        return false;
    }
    //size and set count match

    return lhs.compareBits(rhs) < 0;
}

bool operator==(const DynamicBitset& lhs, const DynamicBitset& rhs){
    if(lhs.size() != rhs.size()){
        return false;
    }

    if(lhs.getSetCount() != rhs.getSetCount()){
        return false;
    }

    return lhs.compareBits(rhs) == 0;
}