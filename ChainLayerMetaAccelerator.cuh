#pragma once

#include "pch.h"

#include <vector>

#include "GeometryLib/LineSegment.cuh"


NVCC_D inline bool checkCollisions(const LineSegment* const segmentsList,
    unsigned int a1Index, unsigned int a2Index);

NVCC_G void precacheChains(char* const bitTable, const LineSegment* const segmentsList,
    const unsigned int segmentsQty, const unsigned int rowWidth);

class PreCache{
    const char* c;
    unsigned int size;
public:
    PreCache(void);
    PreCache(const char* const c, const unsigned int s);
    PreCache& operator=(PreCache&& other);
    ~PreCache(void);

    bool at(const unsigned int i, const unsigned int j) const;

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