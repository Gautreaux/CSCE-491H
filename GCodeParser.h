#pragma once

#include <string>
#include <vector>

#include "pch.h"

#include "GCodeSegment.h"
#include "GeometryLib/Point3.h"

typedef std::vector<GCodeSegment>::const_iterator SegmentIterator;
typedef std::vector<double>::const_iterator LayerIterator;

#define SPLIT_TARGET_MM 25

class GCodeParser{
private:
    // required subclasses
    enum class GCodeUnits {INCHES, MILLIMETERS};
    enum class GCodePositioningType {ABSOLUTE, RELATIVE};
    enum class ErrorTypes {PARSE_ERROR, MONO_ERROR, Z_ERROR, CONTINUOUS_ERROR};
    struct SplitToken{char letter; double number;};

    //data members
    unsigned char errorFlags; //bitflags of error conditions
    unsigned int fileSize;
    std::vector<GCodeSegment> originalSegmentsList;
    std::vector<GCodeSegment> segmentsList; 
    std::vector<double> zLayers; // set of zLayers with print elements

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

    //return an uchar type for the 
    //  should NOT return a value >= 8
    unsigned char convertParseValid(ErrorTypes p) const;

    //set the error bit that coressponds to the value
    inline void setErrorBit(ErrorTypes p){errorFlags |= (1 << convertParseValid(p));}

    //return true if the coressponding error bit is set
    inline bool readErrorBit(ErrorTypes p) const {return (errorFlags & (1 << convertParseValid(p))) != 0;}
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
    inline unsigned int numberOrigSegments(void) const {return originalSegmentsList.size();}
    inline unsigned int numberSegments(void) const {return segmentsList.size();}
    inline unsigned int numberZLayers(void) const {return zLayers.size();}
    inline unsigned int numberSegmentsInLayer(const double d) const {return (getLayerEndIndex(d) - getLayerStartIndex(d) + 1);} //+1 because inclusive
    inline unsigned int numberOrigSegmentsInLayer(const double d) const {return (getLayerOrigEndIndex(d) - getLayerOrigStartIndex(d) + 1);} //+1 because inclusive

    //return the validity of the file
    bool inline isValid(void) const {return errorFlags == 0;}
    bool inline parseError(void) const {return readErrorBit(ErrorTypes::PARSE_ERROR);}
    bool inline monoError(void) const {return readErrorBit(ErrorTypes::MONO_ERROR);}
    bool inline zError(void) const {return readErrorBit(ErrorTypes::Z_ERROR);}
    bool inline continuousError(void) const {return readErrorBit(ErrorTypes::CONTINUOUS_ERROR);}
    operator bool() const {return isValid();}

    inline unsigned int getFileSize(void) const {return fileSize;}

    //iterators and access
    const GCodeSegment& at(unsigned int i) const;
    const GCodeSegment& orig_at(unsigned int i) const;
    inline SegmentIterator segments_begin() const {return segmentsList.begin();}
    inline SegmentIterator segments_end() const {return segmentsList.end();}
    inline SegmentIterator origsegments_begin() const {return originalSegmentsList.begin();}
    inline SegmentIterator origsegments_end() const {return originalSegmentsList.end();}
    inline LayerIterator layers_begin() const {return zLayers.begin();}
    inline LayerIterator layers_end() const {return zLayers.end();}

    //get a const ref to the vector of layers
    inline const std::vector<double>& getLayerVecRef() const {return zLayers;}

    // get the start/end of a zLayer
    unsigned int getLayerStartIndex(double zLayerTarget, unsigned int hint = 0) const;
    unsigned int getLayerEndIndex(double zLayerTarget, unsigned int hint = -1) const;

    unsigned int getLayerOrigStartIndex(double zLayerTarget, unsigned int hint = 0) const;
    unsigned int getLayerOrigEndIndex(double zLayerTarger, unsigned int hint = -1) const;
};

template <class Functor>
void iterateGCPLayer(const GCodeParser& gcp, double zLayer, Functor& functor){
    for(unsigned int i = gcp.getLayerStartIndex(zLayer); i <= gcp.getLayerEndIndex(zLayer); i++){
        functor(gcp.at(i));
    }
}

template <class Functor>
void iterateOrigGCPLayer(const GCodeParser& gcp, double zLayer, Functor& functor){
    for(unsigned int i = gcp.getLayerOrigStartIndex(zLayer); i <= gcp.getLayerOrigEndIndex(zLayer); i++){
        functor(gcp.orig_at(i));
    }
}

template <class Functor>
void iterateGCPSlice(const GCodeParser& gcp, const unsigned int startIndex, const unsigned int endIndex, Functor& functor){
    for(auto i = startIndex; i <= endIndex; i++){
        functor(gcp.at(i));
    }
}

template <class Functor>
void iterateOrigGCPSlice(const GCodeParser& gcp, const unsigned int startIndex, const unsigned int endIndex, Functor& functor){
    for(auto i = startIndex; i <= endIndex; i++){
        functor(gcp.orig_at(i));
    }
}