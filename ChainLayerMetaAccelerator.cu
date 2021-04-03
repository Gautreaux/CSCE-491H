#include "ChainLayerMetaAccelerator.cuh"

NVCC_D bool checkCollisions(const LineSegment* const segmentsList,
    unsigned int a1Index, unsigned int a2Index)
{
    return DOUBLE_GEQ(segmentsList[a1Index].minSeperationDistance(segmentsList[a2Index]), 25.0);
}

NVCC_G void precacheChains(char* const bitTable, const LineSegment* const segmentsList,
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

template<typename CLM_Type>
void offloadPrecaching(CLM_Type* const clm){
    const unsigned int numberPrintSegments = clm->getNumberPrintSegmentsInLayer();
    const unsigned int rowWidth = CEIL_DIVISION(numberPrintSegments, sizeof(char));
    std::vector<LineSegment> segmentsList;
    segmentsList.reserve(numberPrintSegments);
    for(unsigned int i = 0; i < numberPrintSegments; i++){
        segmentsList.push_back(clm->getSegmentByLayerIndex(i));
    }

    LineSegment* segmentsList_device;
    const unsigned int sizeofSegListBytes = sizeof(LineSegment)*numberPrintSegments;

    auto e = cudaMalloc((void**)&segmentsList_device, sizeofSegListBytes);
    if(e == cudaErrorMemoryAllocation){
        std::cout << "Device OOM" << std::endl;
    }
    if(e != cudaSuccess){
        std::cout << "Error occurred in CUDA malloc: " << e << std::endl;
        exit(e);
    };

    cudaMemcpy(segmentsList_device, segmentsList.data(), sizeofSegListBytes, cudaMemcpyHostToDevice);




    cudaFree(segmentsList_device);
}
