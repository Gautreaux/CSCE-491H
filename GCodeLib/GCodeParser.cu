
#include <fstream>
#include <sstream>
#include <vector>

#include "GCodeParser.h"
#include "../pch.h"

GCodeParser::GCodeParser(const std::string filePath) : errorFlags(0), fileSize(0), filePath(filePath) {
    // setup the object

    // do the file parsing
    bool valid = parseFile(filePath);
#ifdef DEBUG
    printf("Parsing %s for file %s\n", valid ? "successful" : "failed", filePath.c_str());
#endif
    if(valid == false){
        setErrorBit(ErrorTypes::PARSE_ERROR);
        return;
    }

    // simple checks
    //  this could be more efficient, but whatever
    //  the checks do a decent amount of duplicate work
    if(isMonotonicIncreasingZ() == false){
        setErrorBit(ErrorTypes::MONO_ERROR);
        valid = false;
    }
    if(isZSlicedPrint() == false){
        setErrorBit(ErrorTypes::Z_ERROR);
        valid = false;
    }
    if(isContinuousPrint() == false){
        setErrorBit(ErrorTypes::CONTINUOUS_ERROR);
        valid = false;
    }
#ifdef DEBUG
    printf("Post-Parse Checks: %s\n", valid ? "Passing" : "Failed");
#endif      
    if(valid == false){
#ifdef DEBUG
        // not the most efficient, but shouldn't be run so thats cool
        //  also, the compiler shouldn't recognize/cache results?
        // TODO - rewrite to use the error bits and problem is solved
        printf("\tMonotonic increasing z: %s\n", isMonotonicIncreasingZ() ? "Passing" : "Failed");
        printf("\tZ-Sliced Print: %s\n", isZSlicedPrint() ? "Passing" : "Failed");
        printf("\tContinuous Print: %s\n", isContinuousPrint() ? "Passing" : "Failed");
#endif
        return;
    }


    // build the zLayers
    assert(readErrorBit(ErrorTypes::Z_ERROR) == false);
    zLayers.empty();
    zLayers.push_back(segmentsList[0].getStartPoint().getZ());
    for(auto segment : segmentsList){
        if(segment.isPrintSegment()){
            assert(segment.isZParallel()); // implied by Z_ERROR bit
            if(zLayers.size() == 0 || (segment.getStartPoint().getZ() > zLayers.back())){ 
                zLayers.push_back(segment.getStartPoint().getZ());
            }
        }
    }

    //now we want to split the print segments 
    originalSegmentsList = std::move(segmentsList);
    for(auto& originalSeg : originalSegmentsList){
        if(originalSeg.isPrintSegment() == false){
            segmentsList.push_back(originalSeg);
            continue;
        }
        if(originalSeg.length() <= SPLIT_TARGET_MM){
            //this segment is already short enough, so skip it
            segmentsList.push_back(originalSeg);
            continue;
        }
        
        double origLen = originalSeg.length();
        //check, should be guaranteed above
        assert(originalSeg.isZParallel());

        const Vector3 stepAmount = originalSeg.getSlope().scaleVector(SPLIT_TARGET_MM);
        const Point3& startPoint = originalSeg.getStartPoint();
        Point3 lastPoint = startPoint;
        double printPerUnit = originalSeg.getPrintAmount() / origLen;
        
        do{
            Point3 newPoint = lastPoint + stepAmount;
            if(getPointDistance(startPoint, newPoint) > origLen){
                //last segment
                double thisLen = getPointDistance(lastPoint, originalSeg.getEndPoint());
                segmentsList.push_back(GCodeSegment(lastPoint, originalSeg.getEndPoint(), thisLen*printPerUnit));
                break;
            }else{
                //partial segment
                assert(DOUBLE_EQUAL(getPointDistance(lastPoint, newPoint), SPLIT_TARGET_MM));
                segmentsList.push_back(GCodeSegment(lastPoint, newPoint, printPerUnit*SPLIT_TARGET_MM));
                lastPoint = newPoint;
            }
        }while(true);
    }
}

bool GCodeParser::parseFile(const std::string filePath){
    std::ifstream infile(filePath);
    if(infile.is_open() == false){
#ifdef DEBUG
        printf("File for path %s failed to open.\n", filePath.c_str());
#endif
        return false;
#ifdef DEBUG
    }else{
        printf("File for path %s opened successfully\n", filePath.c_str());
#endif
    }

    //seems like this shouldn't be necessary
    infile.clear();
    infile.seekg(0, std::ios::beg);
    auto b = infile.tellg();
    infile.seekg(0, std::ios::end);
    fileSize = infile.tellg() - b;

    if(fileSize > (MAX_FILE_SIZE_MB * MB_TO_B)){
        setErrorBit(ErrorTypes::FILE_TOO_LARGE);
        return false;
    }

    //return to beginning
    infile.seekg(0, std::ios::beg);

    //do the meat of the processing
    std::string line;
    std::vector<std::string> lineTokens;
    lineTokens.reserve(20); // attempt to prevent resizing later
#ifdef DEBUG
    int lineCounter = 0; //so we can see the problematic line in gdb
#endif

    GCodeUnits currentUnits = GCodeUnits::MILLIMETERS; //TODO - should probably start as some "not-set" until file provides a unit
    GCodePositioningType currentPositioningType = GCodePositioningType::ABSOLUTE; // TODO ^
    double posX = 0; // TODO ^
    double posY = 0; // TODO ^
    double posZ = 0; // TODO ^
    double posE = 0; // TODO ^

    while (std::getline(infile, line)){
#ifdef DEBUG
        lineCounter++;
#endif
        if(line == ""){
            continue;
        }

        std::istringstream lineStream(line);
        std::string token;
        lineTokens.clear(); // remove the previous line's tokens

        //break the line apart into its tokens
        while(lineStream >> token){
            if(token == ";"){
                //a comment has been started,
                //  the rest of the line will be ignored, so we can skip it
                break;
            }
            
            lineTokens.push_back(token);

        }

        if(lineTokens.size() == 0){
            //this entire line is a comment
            continue;
        }

        //now we will process on the various tokens
        auto split = splitToken(lineTokens[0]);
        if (split.letter == 'G'){
            // some form of a move command
            if (split.number == 20){
                // G20 - switch units type to inches
                currentUnits = GCodeUnits::INCHES;
                throw UnrecognizedCommandException(line, lineTokens[0]);
            }else if (split.number == 21){
                // G21 - switch units type to millimeters
                currentUnits = GCodeUnits::MILLIMETERS;
            }else if (split.number == 90){
                // G90 - switch positioning type to absolute
                currentPositioningType = GCodePositioningType::ABSOLUTE;
            }else if (split.number == 91){
                // G91 - switch positioning type to relative
                currentPositioningType = GCodePositioningType::RELATIVE;
            }else if (split.number == 28){
                // G28 - home axis
                //  with no arguments, home all the axis
                //  with arguments, home the ones specified
                if(lineTokens.size() == 1){
                    // no arguments, home all
                    posX = 0;
                    posY = 0;
                    posZ = 0;
                    posE = 0;
                }
                else{
                    for(unsigned int i = 1; i < lineTokens.size(); i++){
                        //TODO - i think that you can home axis to positions
                        //  line "G28 X5" is valid
                        auto st = splitToken(lineTokens[i]);
                        if(st.letter == 'X'){
                            posX = 0;
                        }else if(st.letter == 'Y'){
                            posY = 0;
                        }else if(st.letter == 'Z'){
                            posZ = 0;
                        }else if(st.letter == 'E'){
                            posE = 0;
                        }
                        else{
                            //just ignore any other axis homing that may need to occur
                            // such as the F axis
                        }
                    }
                }
            }else if(split.number == 1){
                if(currentUnits == GCodeUnits::INCHES){
                    //this should be more descriptive but whatever
                    throw UnrecognizedCommandException(line, lineTokens[0]);
                }

                //G1 - standard liner interpolation move
                double startX = posX;
                double startY = posY;
                double startZ = posZ;
                double startE = posE;

                for(unsigned int i = 1; i < lineTokens.size(); i++){
                    auto st = splitToken(lineTokens[i]);
                    if(st.letter == 'X'){
                        posX = (currentPositioningType == GCodePositioningType::ABSOLUTE) ? st.number : posX + st.number;
                    }else if(st.letter == 'Y'){
                        posY = (currentPositioningType == GCodePositioningType::ABSOLUTE) ? st.number : posY + st.number;
                    }if(st.letter == 'Z'){
                        posZ = (currentPositioningType == GCodePositioningType::ABSOLUTE) ? st.number : posZ + st.number;
                    }if(st.letter == 'E'){
                        posE = (currentPositioningType == GCodePositioningType::ABSOLUTE) ? st.number : posE + st.number;
                    }else{
                        // nothing to do on this input type
                        //  EX: F - setting the feed rate to 
                    }
                }

                //construct the actual line segment object
                Point3 startPoint(startX, startY, startZ);
                Point3 endPoint(posX, posY, posZ);
                double printAmount = posE - startE;

                if(startPoint == endPoint){
                    //no-move command
                    //just pass without adding this as a segment
                }
                else{
                    //prune the segments list to the first print segment
                    GCodeSegment thisSeg(startPoint, endPoint, printAmount);
                    if(thisSeg.isPrintSegment() || segmentsList.size() > 0){
                        segmentsList.push_back(thisSeg);
                    }
                    //segmentsList.emplace_back(startPoint, endPoint, printAmount);
                }
                
            }else{
                //this was an unrecognized command, so we give up
                throw UnrecognizedCommandException(line, lineTokens[0]);
            }
        }else if (split.letter == 'M'){
            // a M command,
            // for the moment we just skip these
            // probably dont want to just blanket ignore, but it seems to be fine
        }else{
            //this was an unrecognized command, so we give up
            throw UnrecognizedCommandException(line, lineTokens[0]);
        }
        
    }

    //ifstream destructor should call this automatically?
    infile.close();

    //parsing completes successfully
    return true;
}

GCodeParser::SplitToken GCodeParser::splitToken(const std::string token){
    GCodeParser::SplitToken toReturn;
    toReturn.letter = toupper(token[0]);
    toReturn.number = std::stod(token.substr(1));
    return toReturn;
}

GCodeParser::UnrecognizedCommandException::UnrecognizedCommandException(const std::string line, const std::string token):
        problemLine(line), problemToken(token) 
{
};

bool GCodeParser::isMonotonicIncreasingZ(void) const {
    double lastZ = segmentsList[0].getStartPoint().getZ();

    for(auto segment : segmentsList){
        if(lastZ > segment.getStartPoint().getZ()){
            return false;
        }else if(lastZ < segment.getStartPoint().getZ()){
            lastZ = segment.getStartPoint().getZ();
        }

        if(lastZ > segment.getEndPoint().getZ()){
            return false;
        }else if(lastZ < segment.getEndPoint().getZ()){
            lastZ = segment.getEndPoint().getZ();
        }
    }
    return true;
}

bool GCodeParser::isZSlicedPrint(void) const {
    for(auto segment : segmentsList){
        if(segment.isPrintSegment() == true && segment.isZParallel() == false){
            return false;
        }
    }
    return true;
}

bool GCodeParser::isContinuousPrint(void) const {
    Point3 lastPos = segmentsList[0].getStartPoint();

    for(auto segment : segmentsList){
        if(segment.getStartPoint() != lastPos){
            return false;
        }
        lastPos = segment.getEndPoint();
    }
    return true;
}

unsigned char GCodeParser::convertParseValid(GCodeParser::ErrorTypes p) const {
    switch (p)
    {
    case ErrorTypes::PARSE_ERROR:
        return 0;
    case ErrorTypes::MONO_ERROR:
        return 1;
    case ErrorTypes::Z_ERROR:
        return 2;
    case ErrorTypes::CONTINUOUS_ERROR:
        return 3;
    case ErrorTypes::FILE_TOO_LARGE:
        return 4;
    }
    throw std::exception();
};

const GCodeSegment& GCodeParser::at(unsigned int i) const{
    if(i < 0 || i > numberSegments()){
        throw std::out_of_range("");
    }

    return segmentsList[i];
}

const GCodeSegment& GCodeParser::orig_at(unsigned int i) const{
    if(i < 0 || i > numberOrigSegments()){
        throw std::out_of_range("");
    }

    return segmentsList[i];
}

unsigned int GCodeParser::getLayerStartIndex(double zLayerTarget, unsigned int hint) const {
    //TODO - could be a binary search or something more efficient, because
    //requires segments to be in monotonic increasing order
    assert(readErrorBit(ErrorTypes::MONO_ERROR) == false);

    for(unsigned int i = hint; i < segmentsList.size(); i++){
        const GCodeSegment& gcs = segmentsList.at(i);
        if(gcs.isZParallel()){
            double z = gcs.getStartPoint().getZ();
            if(DOUBLE_EQUAL(z, zLayerTarget)){
                return i;
            }else if(z > zLayerTarget){
                break;
            }
        }
    }

    throw std::runtime_error("Could not match provided zLayer to one in file");
}


unsigned int GCodeParser::getLayerEndIndex(double zLayerTarget, unsigned int hint) const {
    //TODO - could be a binary search or something more efficient, because
    //requires segments to be in monotonic increasing order
    assert(readErrorBit(ErrorTypes::MONO_ERROR) == false);

    hint = ((hint < segmentsList.size()-1) ? hint : (segmentsList.size() - 1));

    for(int i = hint; i >= 0; i--){
        const GCodeSegment& gcs = segmentsList.at(i);
        if(gcs.isZParallel()){
            auto z = gcs.getStartPoint().getZ();
            if(z == zLayerTarget){
                return i;
            }else if(z < zLayerTarget){
                break;
            }
        }
    }

    throw std::runtime_error("Could not match provided zLayer to one in file");
}

unsigned int GCodeParser::getLayerOrigStartIndex(double zLayerTarget, unsigned int hint) const {
    //TODO - could be a binary search or something more efficient, because
    //requires segments to be in monotonic increasing order
    assert(readErrorBit(ErrorTypes::MONO_ERROR) == false);

    for(unsigned int i = hint; i < originalSegmentsList.size(); i++){
        const GCodeSegment& gcs = originalSegmentsList.at(i);
        if(gcs.isZParallel()){
            auto z = gcs.getStartPoint().getZ();
            if(z == zLayerTarget){
                return i;
            }else if(z > zLayerTarget){
                break;
            }
        }
    }

    throw std::runtime_error("Could not match provided zLayer to one in file");
}


unsigned int GCodeParser::getLayerOrigEndIndex(double zLayerTarget, unsigned int hint) const {
    //TODO - could be a binary search or something more efficient, because
    //requires segments to be in monotonic increasing order
    assert(readErrorBit(ErrorTypes::MONO_ERROR) == false);

    hint = ((hint < originalSegmentsList.size()-1) ? hint : (originalSegmentsList.size() - 1));

    for(int i = hint; i >= 0; i--){
        const GCodeSegment& gcs = originalSegmentsList.at(i);
        if(gcs.isZParallel()){
            auto z = gcs.getStartPoint().getZ();
            if(z == zLayerTarget){
                return i;
            }else if(z < zLayerTarget){
                break;
            }
        }
    }

    throw std::runtime_error("Could not match provided zLayer to one in file");
}