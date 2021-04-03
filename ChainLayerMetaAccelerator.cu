#include "ChainLayerMetaAccelerator.cuh"

NVCC_D bool checkCollisions(const LineSegment* const segmentsList,
    unsigned int a1Index, unsigned int a2Index)
{
    // if(a1Index == 0 && a2Index == 4){
    //     bool b = DOUBLE_GEQ(segmentsList[a1Index].minSeperationDistance(segmentsList[a2Index]), 25.0);
    //     printf("CUDA i = 0, j = 4: %d\n", b);
    // }
    return DOUBLE_GEQ(segmentsList[a1Index].minSeperationDistance(segmentsList[a2Index]), 25.0);
}

NVCC_G void precacheChains(char* const bitTable, const LineSegment* const segmentsList,
    const unsigned int segmentsQty)
{
    const unsigned int mySegentIndex = threadIdx.x + blockIdx.x * blockDim.x;
    if(mySegentIndex >= segmentsQty){
        return;
    }
    // printf("Starting index: %d\n", mySegentIndex);
    unsigned int ctr = 0;
    for(unsigned int thisRow = 0; thisRow < CEIL_DIVISION(segmentsQty, sizeof(char)*8); thisRow++){
        char c = 0;
        for(unsigned int j = 0; j < sizeof(char)*8; j++, ctr++){
            if(ctr >= segmentsQty){
                continue;
            }

            // printf("CALC: %d %d\n", mySegentIndex, ctr);

            bool b = checkCollisions(segmentsList, mySegentIndex, ctr);
            c |= (((b) ? 1 : 0) << j);

            // if(mySegentIndex == 0 && (ctr == 4 || ctr == 32 || ctr == 162 || ctr == 196)){
            //     printf("CUDA err: i = %d, j = %d, offset = %d, bool: %d, segmentsQty: %d, thisRow: %d,"
            //             "mySegmentIndex: %d, storeIndex: %d, ctr: %d \n",
            //             mySegentIndex, ctr, j, b, segmentsQty, thisRow, 
            //             mySegentIndex, segmentsQty*thisRow + mySegentIndex,
            //             ctr);
            // }
        }
        // int d = (int)c;
        // printf("CUDA set: %d %d\n", segmentsQty*thisRow + mySegentIndex, d);
        bitTable[segmentsQty*thisRow + mySegentIndex] = c;
    }
}

PreCache offloadPrecaching(
    const unsigned int numberPrintSegments,
    const std::vector<LineSegment>& segmentsList
){
    assert(numberPrintSegments == segmentsList.size());

    const unsigned int numberRows = CEIL_DIVISION(numberPrintSegments, sizeof(char)*8);

    LineSegment* segmentsList_device;
    const unsigned int sizeofSegListBytes = sizeof(LineSegment)*numberPrintSegments;

    char* outputList_device;
    const unsigned int sizeofOutputBytes = numberRows * numberPrintSegments;

    std::cout << "Attempting cudaMalloc of segments/output: " << sizeofSegListBytes << " " << sizeofOutputBytes << std::endl;


    for(unsigned int i = 0; i < 2 ; i++){
        auto e = cudaMalloc(
            ((i == 0) ? ((void**)&segmentsList_device) : (void**)&outputList_device),
            ((i == 0) ? sizeofSegListBytes : sizeofOutputBytes)
        );
        if(e == cudaErrorMemoryAllocation){
            std::cout << "Device OOM?" << std::endl;
        }
        if(e != cudaSuccess){
            std::cout << "Error occurred in CUDA malloc: " << e << std::endl;
            printf("Reason: %s\n", cudaGetErrorString(e));
            exit(e);
        };
    }

    cudaMemcpy(segmentsList_device, segmentsList.data(), sizeofSegListBytes, cudaMemcpyHostToDevice);
    cudaMemset(outputList_device, 0, sizeofOutputBytes);

    const unsigned int threadsPerBlock = 256;
    const unsigned int numberBlocks =  CEIL_DIVISION(numberPrintSegments, threadsPerBlock);

    precacheChains<<<numberBlocks, threadsPerBlock>>>(outputList_device, segmentsList_device, numberPrintSegments);

    auto e = cudaDeviceSynchronize();
    if(e != cudaSuccess){
        printf("CUDA synchronize failed with %d:%s\n",
            e, cudaGetErrorString(e));
        exit(e);
    }

    char* const outputList = (char*)malloc(sizeofOutputBytes);
    cudaMemcpy(outputList, outputList_device, sizeofOutputBytes, cudaMemcpyDeviceToHost);

    // for(unsigned int i = 0; i < sizeofOutputBytes; i++){
    //     int k;
    //     k = (int)(outputList[i]);
    //     printf("HOST read: %d %d\n", i, k);
    // }

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

    printf("!!!REMOVE ME!!!\n");
    e = cudaDeviceSetLimit(cudaLimitPrintfFifoSize, 1*1024*1024*1024);
    if(e != 0){
        printf("set limit CUDA error #%d: %s\n", e, cudaGetErrorString(e));
    }
    size_t mySize;
    cudaDeviceGetLimit(&mySize, cudaLimitPrintfFifoSize);
    std::cout << "Resolved printf size: " << mySize << std::endl;

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