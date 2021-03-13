
#include "ChainStarLog.h"

void dumpChainPairsToFile(
    std::string basename, 
    const std::vector<PreComputeChain>& chainPairs,
    const ChainLayerMeta& clm,
    const double subname
){
    { //keep t scoped so as to not pollute later
        size_t t;
        while ((t = basename.find('/')) != std::string::npos)
        {
            basename = basename.substr(t+1);
        }
    }

    if(basename.substr(basename.length()-strlen(GCODE_EXTENSION)) == GCODE_EXTENSION){
        basename = basename.substr(0, basename.length()-strlen(GCODE_EXTENSION));
    }

    const std::string filename = (
        basename + 
        ((subname == MIN_DOUBLE) ? "" : ("." + std::to_string(subname))) +
        CHAIN_DUMP_EXTENSION
    );

    std::cout << "Dumping layer meta to file: " << filename << std::endl;

    std::ofstream ofs(filename);
    if(ofs.is_open() == false){
        std::cout << "Failed to open chain dump file : " << filename << std::endl;
        throw 0;
    }

    for(auto chainPair : chainPairs){
        ofs << chainPair.amountPrinted << " ";

        if(chainPair.c1.isNoopChain()){
            ofs << "NOOP NOOP";
        }else{
            ofs << clm.getChainStartPoint(chainPair.c1) << " ";
            ofs << clm.getChainEndPoint(chainPair.c1) << " ";
        }

        if(chainPair.c2.isNoopChain()){
            ofs << "NOOP NOOP";
        }else{   
            ofs << clm.getChainStartPoint(chainPair.c2) << " ";
            ofs << clm.getChainEndPoint(chainPair.c2) << " ";
        }
        ofs << std::endl;
    }

    // std::cout << "\n\n\n\nDUMP\n\n\n\n" << std::endl;

}