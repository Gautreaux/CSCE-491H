#pragma once

#include "pch.h"

#include <vector>

#include "GeometryLib/LineSegment.cuh"


NVCC_D inline bool checkCollisions(const LineSegment* const segmentsList,
    unsigned int a1Index, unsigned int a2Index);

NVCC_G void precacheChains(char* const bitTable, const LineSegment* const segmentsList,
    const unsigned int segmentsQty, const unsigned int rowWidth);

template<typename CLM_Type>
void offloadPrecaching(CLM_Type* const clm);