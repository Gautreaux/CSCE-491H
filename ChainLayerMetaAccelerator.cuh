#pragma once

#include "pch.h"

#include <vector>

#include "GeometryLib/LineSegment.cuh"


NVCC_D inline bool checkCollisions(const LineSegment* const segmentsList,
    unsigned int a1Index, unsigned int a2Index);

NVCC_G void precacheChains(char* const bitTable, const LineSegment* const segmentsList,
    const unsigned int segmentsQty);

class PreCache{
    const char* c;
    unsigned int size;
public:
    PreCache(void);
    PreCache(const char* const c, const unsigned int s);
    PreCache& operator=(PreCache&& other);
    ~PreCache(void);

    inline bool at(const unsigned int i, const unsigned int j) const {
        const unsigned int rowNumber = j / (sizeof(char)*8);
        const unsigned int offset = j % (sizeof(char)*8);

        // printf("i = %d, j = %d, row# = %d, offset = %d, index = %d, bool: %d\n", 
        //     i, j, rowNumber, offset, rowNumber*size + i, ((c[rowNumber*size + i] & (1 << offset)) != 0)
        // );

#ifdef NVCC_DEBUG
        assert(rowNumber < CEIL_DIVISION(size, 8));
#endif
        
        return (c[rowNumber*size + i] & (1 << offset)) != 0;
    };

    void markEmpty(void){
        c = nullptr;
        size = 0;
    }

};

PreCache offloadPrecaching(
    const unsigned int numberPrintSegments,
    const std::vector<LineSegment>& segmentsList
);

void logCUDAInfo(void);