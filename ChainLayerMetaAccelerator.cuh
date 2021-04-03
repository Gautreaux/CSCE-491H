#pragma once

#include "pch.h"

#include <vector>

#include "GeometryLib/LineSegment.cuh"


NVCC_D inline bool checkCollisions(const LineSegment* const segmentsList,
    unsigned int a1Index, unsigned int a2Index);

NVCC_G void precacheChains(char* const bitTable, const LineSegment* const segmentsList,
    const unsigned int segmentsQty, const unsigned int rowWidth);

void offloadPrecaching(
    const unsigned int numberPrintSegments,
    const std::vector<LineSegment>& segmentsList
);

void logCUDAInfo(void);