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

PreCache offloadPrecaching(
    const unsigned int numberPrintSegments,
    const std::vector<LineSegment>& segmentsList
){
    assert(numberPrintSegments == segmentsList.size());

    const unsigned int rowWidth = CEIL_DIVISION(numberPrintSegments, sizeof(char));

    LineSegment* segmentsList_device;
    const unsigned int sizeofSegListBytes = sizeof(LineSegment)*numberPrintSegments;

    char* outputList_device;
    const unsigned int sizeofOutputBytes = rowWidth * numberPrintSegments;

    std::cout << "Attempting cudaMalloc of segments/output: " << sizeofSegListBytes << " " << sizeofOutputBytes << std::endl;


    for(unsigned int i = 0; i < 2 ; i++){
        auto e = cudaMalloc(
            ((i == 0) ? ((void**)&segmentsList_device) : (void**)&outputList_device),
            ((i == 0) ? sizeofSegListBytes : sizeofOutputBytes)
        );
        if(e == cudaErrorMemoryAllocation){
            std::cout << "Device OOM" << std::endl;
        }
        if(e != cudaSuccess){
            std::cout << "Error occurred in CUDA malloc: " << e << std::endl;
            printf("%s\n", cudaGetErrorString(e));
            exit(e);
        };
    }

    cudaMemcpy(segmentsList_device, segmentsList.data(), sizeofSegListBytes, cudaMemcpyHostToDevice);

    const unsigned int threadsPerBlock = 256;
    const unsigned int numberBlocks =  CEIL_DIVISION(numberPrintSegments, threadsPerBlock);

    precacheChains<<<numberBlocks, threadsPerBlock>>>(outputList_device, segmentsList_device, numberPrintSegments, rowWidth);

    char* const outputList = (char*)malloc(sizeofOutputBytes);
    cudaMemcpy(outputList, outputList_device, sizeofOutputBytes, cudaMemcpyDeviceToHost);

    cudaFree(segmentsList_device);
    cudaFree(outputList_device);

    return PreCache(outputList, numberPrintSegments);
}

void logCUDAInfo(void){
    std::cout << "=============================" << std::endl;
    std::cout << "CUDA info:" << std::endl;
    
    int driverVersion, runtimeVersion, deviceCount;

    auto e = cudaRuntimeGetVersion(&runtimeVersion);
    if(e != 0){
        printf("get runtime CUDA error #%d: %s\n", e, cudaGetErrorString(e));
    }
    std::cout << "CUDA Runtime Version: " << runtimeVersion << std::endl;
    e = cudaDriverGetVersion(&driverVersion);
    if(e != 0){
        printf("get version CUDA error #%d: %s\n", e, cudaGetErrorString(e));
    }
    std::cout << "CUDA Driver Version: " << driverVersion << std::endl;
    e = cudaGetDeviceCount(&deviceCount);
    if(e != 0){
        printf("get device CUDA error #%d: %s\n", e, cudaGetErrorString(e));
    }
    std::cout << "CUDA Device Count: " << deviceCount << std::endl;

    std::cout << "=============================" << std::endl;
}

PreCache::PreCache(void) : c(nullptr), size(0)
{};

PreCache::PreCache(const char* const c, const unsigned int s) :
c(c), size(s)
{};

PreCache& PreCache::operator=(PreCache&& other){
    if(c != nullptr){
        free((void*)c);
        c = nullptr;
        size = 0;
    }
    size = other.size;
    c = other.c;

    other.c = nullptr;
    other.size = 0;

    return *this;
}

PreCache::~PreCache(void){
    if(c!=nullptr){
        free((void*)c);
        c = nullptr;
        size = 0;
    }
}

bool PreCache::at(const unsigned int i, const unsigned int j) const{
    return false;
}