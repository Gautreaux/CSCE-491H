#pragma once

#include "pch.h"

#include "GCodeParser.h"
#include "GCodeSegment.h"
#include "GeometryLib/Point3.h"
#include "UtilLib/BiMap.h"


// manages some metadata about the layer
typedef unsigned int GCP_Index;
typedef unsigned int Bitset_Index;
typedef unsigned int Position_Index;
typedef std::vector<Bitset_Index> Adjacent_Bitset_Indexes;

class LayerManager {
private:
    double layer; //what Z layer this manager corresponds to
    unsigned int totalPrintSegments; //number of print segments
    unsigned int totalPositions; //number of positions in this layer
    BiMap<GCP_Index, Bitset_Index> printedSegmentsTranslation;
    BiMap<Point3, Position_Index> pointPositionIndexTranslation;
    std::vector<Adjacent_Bitset_Indexes> adjacentSegments;

public:
    LayerManager(const GCodeParser& gcp, double layer);

    //generic data
    inline const double getLayer(void) const {
        return layer;
    }
    inline const unsigned int getTotalPrintSegments(void) const {
        return totalPrintSegments;
    }
    inline const unsigned int getTotalPositions(void) const {
        return totalPositions;
    }

    // accesors
    inline const GCP_Index getGCPFromBitset(const Bitset_Index i) const {
        return printedSegmentsTranslation.findByB(i)->second;
    }
    inline const Bitset_Index getBitsetFromGCP(const GCP_Index i) const {
        return printedSegmentsTranslation.findByA(i)->second;     
    }
    inline const Point3& getPoint3FromPos(const Position_Index i) const {
        return pointPositionIndexTranslation.findByB(i)->second;
    }
    inline const Position_Index getPosFromPoint3(const Point3& i) const {
        return pointPositionIndexTranslation.findByA(i)->second;
    }
    inline const Adjacent_Bitset_Indexes& getAdjacentSegments(const Position_Index i) const {
        return adjacentSegments.at(i);
    }

};
    
// inline const GCodeSegment& findSegmentByGCPIndex(const GCP_Index i, const GCodeParser& gcp);
// inline const GCodeSegment& findSegmentByBitsetIndex(const Bitset_Index i, const GCodeParser& gcp);