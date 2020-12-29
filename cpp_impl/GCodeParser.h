#pragma once

#include <string>
#include <vector>

#include "pch.h"

#include "GCodeSegment.h"
#include "GeometryLib/Point3.h"

typedef std::vector<GCodeSegment>::const_iterator SegmentIterator;

class GCodeParser{
private:
    // required subclasses
    enum class GCodeUnits {INCHES, MILLIMETERS};
    enum class GCodePositioningType {ABSOLUTE, RELATIVE};
    struct SplitToken{char letter; double number;};

    //data members
    bool valid; //true iff the parsing completed successfully
    std::vector<GCodeSegment> segmentsList; 

    //split the token into its various components 
    SplitToken inline splitToken(const std::string token);

    //filePath - path to the file containing the GCODE to process
    //returns bool, true iff parsing was successful
    //May raise FileNotFoundException
    bool parseFile(const std::string filePath);
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

    //return the validity of the file
    bool inline isValid(void) const {return valid;}
    operator bool() const {return isValid();}

    SegmentIterator begin() const {return segmentsList.begin();}
    SegmentIterator end() const {return segmentsList.end();}
};