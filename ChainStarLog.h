#pragma once

#include "pch.h"

#include <iostream>
#include <limits>
#include <fstream>
#include <string>
#include <vector>

#include "ChainStarHelper.h"
#include "ChainLayerMeta.h"

#define MIN_DOUBLE std::numeric_limits<double>::min()
#define GCODE_EXTENSION ".gcode"
#define CHAIN_DUMP_EXTENSION ".chaindump"

//dump all the chain pairs, and required information,
//  to the file specified
//If subname is provided, will be appended to the base name
//  else (left as MIN_DOUBLE), subname will be omitted
void dumpChainPairsToFile(
    std::string basename, 
    const std::vector<PreComputeChain>& chainPairs,
    const ChainLayerMeta& clm,
    const double subname = MIN_DOUBLE
);