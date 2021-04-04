#pragma once

#include "pch.h"

#include <vector>
#include <fstream>

#include "GeometryLib/LineSegment.cuh"

#include <cuda_runtime.h>

#define ACCELERATOR_MIN_SEPERATION 25.0

#define NUMBER_CUDA_STREAMS 16

NVCC_HD inline bool theoretical_canMoveSegmentPair(
    const LineSegment& a1Seg, const LineSegment& a2Seg,
    const bool isA1Print = false, const bool isA2Print = false
){
    return DOUBLE_GEQ(a1Seg.minSeperationDistance(a2Seg), ACCELERATOR_MIN_SEPERATION);
}

NVCC_HD inline bool codex_canMoveSegmentPair(
    const LineSegment& a1Seg, const LineSegment& a2Seg,
    const bool isA1Print = false, const bool isA2Print = false
){
    return (
        DOUBLE_LEQ(
            std::max(a1Seg.getStartPoint().getX(), a1Seg.getEndPoint().getX()),
            std::min(a2Seg.getStartPoint().getX(), a2Seg.getEndPoint().getX())
        ) &&
        DOUBLE_GEQ(a1Seg.minSeperationDistance(a2Seg), ACCELERATOR_MIN_SEPERATION)
    );
}

NVCC_HD inline bool current_isValidPositionPair(
    const Point3 &a1Pos, const Point3 &a2Pos)
{
    return (
        DOUBLE_EQUAL(a1Pos.getX(), a2Pos.getX()) &&
        DOUBLE_GEQ(a1Pos.getY(), a2Pos.getY()) &&
        DOUBLE_GEQ(getPointDistance(a1Pos, a2Pos), ACCELERATOR_MIN_SEPERATION));
}

NVCC_HD inline bool current_canMoveSegmentPair(
    const LineSegment &a1Seg, const LineSegment &a2Seg,
    const bool isA1Print = false, const bool isA2Print = false)
{
    return (
        current_isValidPositionPair(a1Seg.getStartPoint(), a2Seg.getStartPoint()) &&
        current_isValidPositionPair(a1Seg.getEndPoint(), a2Seg.getEndPoint()) &&
        DOUBLE_GEQ(a1Seg.minSeperationDistance(a2Seg), ACCELERATOR_MIN_SEPERATION));
}

NVCC_HD inline bool relaxed_isValidPositionPair(
    const Point3& a1Pos, const Point3& a2Pos
){
    return (
        DOUBLE_GEQ(getPointDistance(a1Pos, a2Pos), ACCELERATOR_MIN_SEPERATION) &&
        DOUBLE_GEQ(a1Pos.getY(), a2Pos.getY()) &&
        DOUBLE_LEQ(abs(a1Pos.getX() - a2Pos.getX()), 5)
    );
}

NVCC_HD inline bool relaxed_canMoveSegmentPair(
    const LineSegment &a1Seg, const LineSegment &a2Seg,
    const bool isA1Print = false, const bool isA2Print = false)
{
    return (
        relaxed_isValidPositionPair(a1Seg.getStartPoint(), a2Seg.getStartPoint()) &&
        relaxed_isValidPositionPair(a1Seg.getEndPoint(), a2Seg.getEndPoint()) &&
        DOUBLE_GEQ(a1Seg.minSeperationDistance(a2Seg), ACCELERATOR_MIN_SEPERATION));
}

NVCC_G void precacheChains(char* const bitTable, const LineSegment* const segmentsList,
    const unsigned int segmentsQty, const char mode);

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
    const std::vector<LineSegment>& segmentsList,
    const char mode,
    const unsigned int id, std::ostream& outStream
);

void logCUDAInfo(std::ostream& outStream = std::cout);

void cudaInit(void);