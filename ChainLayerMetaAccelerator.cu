#include "ChainLayerMetaAccelerator.cuh"

NVCC_G void precacheChains(char* const bitTable, const LineSegment* const segmentsList,
    const unsigned int segmentsQty, const char mode)
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

            bool b;
            switch (mode)
            {
            case 0: (b = theoretical_canMoveSegmentPair(segmentsList[mySegentIndex], segmentsList[ctr])); break;
            case 1: (b = codex_canMoveSegmentPair(segmentsList[mySegentIndex], segmentsList[ctr])); break;
            case 2: (b = current_canMoveSegmentPair(segmentsList[mySegentIndex], segmentsList[ctr])); break;
            case 3: (b = relaxed_canMoveSegmentPair(segmentsList[mySegentIndex], segmentsList[ctr])); break;
            default: b = false; break;
            }
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
    const std::vector<LineSegment>& segmentsList,
    const char mode,
    const unsigned int id, std::ostream& outStream
){
    assert(numberPrintSegments == segmentsList.size());

    const unsigned int numberRows = CEIL_DIVISION(numberPrintSegments, sizeof(char)*8);

    LineSegment* segmentsList_device;
    const unsigned int sizeofSegListBytes = sizeof(LineSegment)*numberPrintSegments;

    char* outputList_device;
    const unsigned int sizeofOutputBytes = numberRows * numberPrintSegments;

    outStream << "Attempting cudaMalloc of segments/output: " << sizeofSegListBytes << " " << sizeofOutputBytes << std::endl;


    for(unsigned int i = 0; i < 2 ; i++){
        auto e = cudaMalloc(
            ((i == 0) ? ((void**)&segmentsList_device) : (void**)&outputList_device),
            ((i == 0) ? sizeofSegListBytes : sizeofOutputBytes)
        );
        if(e == cudaErrorMemoryAllocation){
            outStream << "Device OOM?" << std::endl;
        }
        if(e != cudaSuccess){
            outStream << "Error occurred in CUDA malloc: " << e << std::endl;
            outStream << "Reason: " << cudaGetErrorString(e) << std::endl;
            throw e;
        };
    }

    cudaMemcpy(segmentsList_device, segmentsList.data(), sizeofSegListBytes, cudaMemcpyHostToDevice);
    cudaMemset(outputList_device, 0, sizeofOutputBytes);

    const unsigned int threadsPerBlock = 256;
    const unsigned int numberBlocks =  CEIL_DIVISION(numberPrintSegments, threadsPerBlock);

    precacheChains<<<numberBlocks, threadsPerBlock>>>(outputList_device, segmentsList_device, numberPrintSegments, mode);

    auto e = cudaDeviceSynchronize();
    if(e != cudaSuccess){
        outStream << "CUDA synchronize failed with " << e << cudaGetErrorString(e) << std::endl;
        throw e;
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

void logCUDAInfo(std::ostream& outStream){
    outStream << "=============================" << std::endl;
    outStream << "CUDA info:" << std::endl;
    
    int driverVersion, runtimeVersion, deviceCount;

    auto e = cudaRuntimeGetVersion(&runtimeVersion);
    if(e != 0){
        outStream << "get runtime CUDA error #" << e << ": " << cudaGetErrorString(e) << std::endl;
    }
    outStream << "CUDA Runtime Version: " << runtimeVersion << std::endl;
    e = cudaDriverGetVersion(&driverVersion);
    if(e != 0){
        outStream << "get version CUDA error #" << e << ": " << cudaGetErrorString(e) << std::endl;
    }
    outStream << "CUDA Driver Version: " << driverVersion << std::endl;
    e = cudaGetDeviceCount(&deviceCount);
    if(e != 0){
        outStream << "get device CUDA error #" << e << ": " << cudaGetErrorString(e) << std::endl;
    }
    outStream << "CUDA Device Count: " << deviceCount << std::endl;

    printf("!!!REMOVE ME!!!\n");
    e = cudaDeviceSetLimit(cudaLimitPrintfFifoSize, 1*1024*1024*1024);
    if(e != 0){
        outStream << "set limit CUDA error #" << e << ": " << cudaGetErrorString(e) << std::endl;
    }
    size_t mySize;
    cudaDeviceGetLimit(&mySize, cudaLimitPrintfFifoSize);
    outStream << "Resolved printf size: " << mySize << std::endl;

    outStream << "=============================" << std::endl;
}

void cudaInit(void){
    
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