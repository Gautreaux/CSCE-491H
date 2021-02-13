#include "ThreadingContents.h"

void dumpGCP(std::ostream& o, const GCodeParser& gcp){
    o << '\t' << gcp.getFileSize() << std::endl;
    o << '\t' << (gcp.isValid() ? "VALID" : "INV");
    if(!gcp.isValid()){
        o << " " << PRINT_BOOL(gcp.parseError()) << " " << PRINT_BOOL(gcp.monoError()) << " " << PRINT_BOOL(gcp.zError()) << " " << PRINT_BOOL(gcp.continuousError()) << " " << PRINT_BOOL(gcp.tooLargeError()) << std::endl;
        o << std::endl; //extra newline to separate records
        return;
    }
    o << std::endl; // ends the valid invalid line
    o << '\t' << gcp.numberOrigSegments() << " " << gcp.numberSegments() << " " << gcp.numberZLayers() << std::endl;
    
    const std::vector<double>& layersRef = gcp.getLayerVecRef();
    unsigned int nextStart = 0;
    unsigned int nextOrigStart = 0;

    for(unsigned int i = 0; i < layersRef.size(); i++){
        double layerValue = layersRef.at(i);
        unsigned int thisStart = nextStart;
        unsigned int thisOrigStart = nextOrigStart;

        if(i == (layersRef.size()-1)){
            //this is the last layer
            nextStart = -1;
            nextOrigStart = -1;
        }else{
            nextStart = gcp.getLayerStartIndex(layersRef.at(i+1), thisStart);
            nextOrigStart = gcp.getLayerOrigStartIndex(layersRef.at(i+1), thisOrigStart); 
        }

        unsigned int thisEnd = gcp.getLayerEndIndex(layerValue, nextStart);
        unsigned int thisOrigEnd = gcp.getLayerOrigEndIndex(layerValue, nextOrigStart);

        LayerSummer seg;
        iterateGCPSlice(gcp, thisStart, thisEnd, seg);

        LayerSummer orig;
        iterateOrigGCPSlice(gcp, thisOrigStart, thisOrigEnd, orig);

        o << '\t' << layerValue << " ";
        o << seg.getNumSegments() << " ";
        o << orig.getNumSegments() << std::endl;

        seg.dump(o);
        o << std::endl;
        orig.dump(o);
        o << std::endl;

    }
    o << std::endl;
}


void dumpPoints(std::ostream& o, const GCodeParser& gcp){
    if(!gcp){
        return;
    }

    for(auto layer : gcp.getLayerVecRef()){
        LayerManager lm(gcp, layer);
        o << "\t " << layer << " " << lm.getTotalPositions() << std::endl;
    }
}

void threadFunction(const unsigned int threadID, CommonThreadParameters *const CTP){
    CTP->threadsRunning.fetch_add(1);
    unsigned int totalProcessed = 0;
    int fileIndex;

    std::string outPath = CTP->outFileRoot + std::to_string(threadID) + ".repOut";
    std::ofstream ofs(outPath.c_str());

    while(((fileIndex = CTP->t->next()) < ((int)CTP->fileNames->size()))){
        totalProcessed += 1;

        std::string fullPath(CTP->rootName);
        fullPath += '/';
        fullPath += CTP->fileNames->at(fileIndex);

        ofs << fullPath << std::endl;
        try {
            GCodeParser gcp(fullPath);

            // dumpGCP(ofs, gcp);
            dumpPoints(ofs, gcp);

            if(gcp){
                CTP->totalValid.fetch_add(1);
                CTP->totalBytesValid.fetch_add(gcp.getFileSize());
            }
        }catch (...){
            ofs << "\tEXCEPTION" << std::endl;
        }

        CTP->totalProcessed.fetch_add(1);
        ofs.flush();
    }

    ofs.close();

    printf("Thread %u processed %u files\n", threadID, totalProcessed);
    CTP->threadsRunning.fetch_sub(1);
    if(CTP->threadsRunning.load() == 0){
        //technically could be run multiple times
        CTP->running.store(false);
    }
}

void reportingFunction(const CommonThreadParameters *const CTP){
    unsigned int approx_sec = 0;
    unsigned int count =  CTP->fileNames->size();
    unsigned long long lastBytes = 0;
    
    while(CTP->running.load() > 0){
        unsigned int total = CTP->totalProcessed.load();    
        unsigned long long bytes = CTP->totalBytesValid.load();
        
        unsigned long long elapsedBytes = (bytes - lastBytes);
        double MBPS = ((double)(elapsedBytes * B_TO_MB))/REPORT_INTERVAL_SEC;

        printf("[%u], Running: %u, Processed %u/%u (%.2f%%), Valid %u, Bytes (valid) %.0fMB (%.2fMB/s)\n", 
            approx_sec, CTP->threadsRunning.load(), total, count, (((double)(total*100))/count),
            CTP->totalValid.load(), bytes * B_TO_MB, MBPS
        );

        //updating fields
        lastBytes = bytes;

        std::this_thread::sleep_for(std::chrono::seconds(REPORT_INTERVAL_SEC));
        approx_sec += REPORT_INTERVAL_SEC;
    }
}