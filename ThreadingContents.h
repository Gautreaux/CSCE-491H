#pragma once

#include "pch.h"

#include <atomic>
#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>

#include "GCodeParser.h"
#include "LayerManager.h"
#include "UtilLib/ThreadsafeIntGen.h"

#ifndef DEFAULT_THREAD_COUNT
#define DEFAULT_THREAD_COUNT 8
#endif

#define REPORT_INTERVAL_SEC 2

struct CommonThreadParameters{
    ThreadsafeIntGen *const t;
    const std::vector<std::string> *const fileNames;
    const char* const rootName;
    const std::string outFileRoot;

    std::atomic<unsigned int> totalProcessed;
    std::atomic<unsigned int> totalValid;
    std::atomic<unsigned long long> totalBytesValid;
    std::atomic<unsigned int> threadsRunning;
    std::atomic<bool> running;

    CommonThreadParameters(ThreadsafeIntGen* t, 
        std::vector<std::string> *const fileNames,
        const char* const rootName, const std::string outFileRoot = "") : 
        t(t), fileNames(fileNames), rootName(rootName), outFileRoot(outFileRoot),
        totalProcessed(0), totalValid(0), totalBytesValid(0), threadsRunning(0), running(true)
    {}
};

#define PRINT_BOOL(b) (b ? '1' : '0')

class LayerSummer{
private:
    unsigned int numPrint;
    unsigned int numNonPrint;
    unsigned int numSegments;
    double printDist;
    double nonPrintDist;
public:
    LayerSummer() : numPrint(0), numNonPrint(0), numSegments(0),
        printDist(0), nonPrintDist(0)
    {}

    void operator()(const GCodeSegment& seg){
        numSegments += 1;
        if(seg.isPrintSegment()){
            numPrint += 1;
            printDist += seg.length();
        }else{
            numNonPrint += 1;
            nonPrintDist += seg.length();
        }
    }

    void dump(std::ostream& o, std::string prefix = "\t\t"){
        o << prefix << numPrint << " ";
        o << numNonPrint << " ";
        o << printDist << " ";
        o << nonPrintDist;
    }

    inline unsigned int getNumSegments(void) const {return numSegments;}
};

void dumpGCP(std::ostream& o, const GCodeParser& gcp);

void dumpPoints(std::ostream& o, const GCodeParser& gcp);

void threadFunction(const unsigned int threadID, CommonThreadParameters *const CTP);

void reportingFunction(const CommonThreadParameters *const CTP);