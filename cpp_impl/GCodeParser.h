#pragma once

#include <string>
#include <vector>

#include "pch.h"

#include "GCodeSegment.h"
#include "GeometryLib/Point3.h"

typedef std::vector<GCodeSegment>::const_iterator SegmentIterator;
typedef std::vector<double>::const_iterator LayerIterator;

// TODO - is this now completely to remove
// class GCodeParser;
//
// struct for sotring the information about a single layer
// struct GCodeLayer{
//     GCodeParser* parser;
//     unsigned int startIndex;
//     unsigned int afterEndIndex;
// };

class GCodeParser{
private:
    // required subclasses
    enum class GCodeUnits {INCHES, MILLIMETERS};
    enum class GCodePositioningType {ABSOLUTE, RELATIVE};
    struct SplitToken{char letter; double number;};

    //data members
    bool valid; //true iff the parsing completed successfully
    std::vector<GCodeSegment> segmentsList; 
    std::vector<double> zLayers; // set of valid zLayers in the vector

    //split the token into its various components 
    SplitToken inline splitToken(const std::string token);

    //filePath - path to the file containing the GCODE to process
    //returns bool, true iff parsing was successful
    //May raise FileNotFoundException
    bool parseFile(const std::string filePath);

    //return True if the segments in the segment list are monotically increasing in Z
    bool isMonotonicIncreasingZ(void) const;

    //return True if all the print segments are parallel to Z = 0
    bool isZSlicedPrint(void) const;

    //return True if all the segments form a continuous chain
    bool isContinuousPrint(void) const;
public:
    //subclasses
    class UnrecognizedCommandException : public std::exception{
        private:
            std::string problemLine;
            std::string problemToken;
        public:
        UnrecognizedCommandException(const std::string line, const std::string token);
        const std::string getLine(void) {return problemLine;}
        const std::string getToken(void) {return problemToken;}
        std::string getMessage(void) {return ("Unrecognized command " + problemToken + " in line " + problemLine);}
    };

    //filePath - path to the file containing the GCODE to process
    GCodeParser(const std::string filePath);

    //get the number of segments that were parsed out
    int numberSegments(void) const {return segmentsList.size();}
    int numberZLayers(void) const {return zLayers.size();}

    //return the validity of the file
    bool inline isValid(void) const {return valid;}
    operator bool() const {return isValid();}

    //iterators and access
    const GCodeSegment& at(unsigned int i) const;
    SegmentIterator segments_begin() const {return segmentsList.begin();}
    SegmentIterator segments_end() const {return segmentsList.end();}
    LayerIterator layers_begin() const {return zLayers.begin();}
    LayerIterator layers_end() const {return zLayers.end();}

    // get the start/end of a zLayer
    unsigned int getLayerStartIndex(double zLayerTarget) const;
    unsigned int getLayerEndIndex(double zLayerTarget) const;
};