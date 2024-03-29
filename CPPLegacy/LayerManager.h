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
    //TODO - should these be some unordered_bimap type?
    BiMap<GCP_Index, Bitset_Index> printedSegmentsTranslation;
    BiMap<Point3, Position_Index> pointPositionIndexTranslation;

    //mapping of Position_Index to Vector[BitSet_Index]
    std::vector<Adjacent_Bitset_Indexes> adjacentSegments;

    //collection of all the positions that start/end a segment
    std::vector<Position_Index> chainStartEndIndexes;
    unsigned int totalChains;

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

    inline const std::vector<Position_Index>& getChainStartEndIndexes(void) const {
        return chainStartEndIndexes;
    }

    //get the total number of chains in this layer
    inline unsigned int getTotalChains(void) const {
        return totalChains;
    }

    //iterators

    // get the iterator to the beginning of the points
    inline std::map<Point3, Position_Index>::const_iterator getPointsStartIterator(void) const{
        return pointPositionIndexTranslation.findByABegin();
    }

    // get iterator to the end of the points
    inline std::map<Point3, Position_Index>::const_iterator getPointsEndIterator(void) const{
        return pointPositionIndexTranslation.findByAEnd();
    }

};
    
// inline const GCodeSegment& findSegmentByGCPIndex(const GCP_Index i, const GCodeParser& gcp);
// inline const GCodeSegment& findSegmentByBitsetIndex(const Bitset_Index i, const GCodeParser& gcp);