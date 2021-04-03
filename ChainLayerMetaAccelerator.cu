#include "pch.h"
#include "GeometryLib/LineSegment.cu"


__device__ __inline__ bool checkCollisions(const LineSegment* const segmentsList,
    unsigned int a1Index, unsigned int a2Index)
{
    return DOUBLE_GEQ(segmentsList[a1Index].minSeperationDistance(segmentsList[a2Index]), 25.0);
}

__global__ void precacheChains(char* const bitTable, const LineSegment* const segmentsList,
    const unsigned int segmentsQty, const unsigned int rowWidth)
{
    const unsigned int mySegentIndex = threadIdx.x + blockIdx.x * blockDim.x;
    unsigned int ctr = 0;
    for(unsigned int thisRow = 0; thisRow < CEIL_DIVISION(segmentsQty, sizeof(char)); thisRow++){
        char c = 0;
        for(unsigned int j = 0; j < sizeof(char); j++, ctr++){
            if(ctr >= segmentsQty){
                continue;
            }
            bool b = checkCollisions(segmentsList, mySegentIndex, ctr);
            c |= (((b) ? 1 : 0) << j);
        }
        bitTable[rowWidth*thisRow + mySegentIndex] = c;
    }
}