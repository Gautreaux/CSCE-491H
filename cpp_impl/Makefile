CXX:=g++ # which compiler to use
CPPFLAGS=# empty default value for GCC flags, can be overriden on command line
CPPSTD=-std=c++11# which c++ version to use

CPP_COMP_COM=$(CXX) $(CPPFLAGS) $(CPPSTD)
CC_COMPLIE_NO_LINK_AUTO=$(CPP_COMP_COM) -c -o $@ $<

EXECUTABLE=Main.exe

# declaring a phony forces the top level to always rebuild
.PHONY: $(EXECUTABLE) Main.o

all : $(EXECUTABLE)

force : clean $(EXECUTABLE)

clean :
	del /S *.o

# pretty much out of date as soon as it is written
manual_all :
	g++ -g -DDEBUG -o $(EXECUTABLE) Main.cpp GCodeSegment.cpp GCodeParser.cpp GeometryLib/Line.cpp GeometryLib/LineSegment.cpp GeometryLib/Point3.cpp GeometryLib/Vector3.cpp

$(EXECUTABLE) : Main.o GCodeParser.o PrunedAStar.o
	$(CPP_COMP_COM) -o $(EXECUTABLE) *.o

Main.o : Main.cpp GCodeParser.o PrunedAStar.o
	$(CC_COMPLIE_NO_LINK_AUTO)

# geometry files 
GCodeParser.o : GCodeParser.cpp GCodeParser.h GCodeSegment.o Point3.o pch.o
	$(CC_COMPLIE_NO_LINK_AUTO)

GCodeSegment.o : GCodeSegment.cpp GCodeSegment.h LineSegment.o pch.o
	$(CC_COMPLIE_NO_LINK_AUTO)

LineSegment.o : GeometryLib/LineSegment.cpp GeometryLib/LineSegment.h Line.o Point3.o pch.o
	$(CC_COMPLIE_NO_LINK_AUTO)

Line.o : GeometryLib/Line.cpp GeometryLib/Line.h Point3.o Vector3.o pch.o
	$(CC_COMPLIE_NO_LINK_AUTO)

Vector3.o : GeometryLib/Vector3.cpp GeometryLib/Vector3.h Point3.o pch.o
	$(CC_COMPLIE_NO_LINK_AUTO)

Point3.o : GeometryLib/Point3.cpp GeometryLib/Point3.h pch.o
	$(CC_COMPLIE_NO_LINK_AUTO)

pch.o : pch.cpp pch.h
	$(CC_COMPLIE_NO_LINK_AUTO)

# a* files
PrunedAStar.o : PrunedAStar.cpp PrunedAStar.h RecomputeState.o GCodeParser.o BiMap.o PriorityQueue.o pch.o
	$(CC_COMPLIE_NO_LINK_AUTO)

RecomputeState.o : RecomputeState.cpp RecomputeState.h DynamicBitset.o Point3.o pch.o
	$(CC_COMPLIE_NO_LINK_AUTO)

# util files
DynamicBitset.o : UtilLib/DynamicBitset.cpp UtilLib/DynamicBitset.h pch.o
	$(CC_COMPLIE_NO_LINK_AUTO)

BiMap.o : UtilLib/BiMap.cpp UtilLib/BiMap.h pch.o
	$(CC_COMPLIE_NO_LINK_AUTO)

PriorityQueue.o : UtilLib/PriorityQueue.cpp UtilLib/PriorityQueue.h pch.o
	$(CC_COMPLIE_NO_LINK_AUTO)

# Uses builtin .cpp -> .o translation instead
#	which is basically this rule anyway
# # automatic rule for building .o files from same name .cpp files
# # $@ - the name of the target of the rule
# # $< - the name of the first prequesite
# # $^ - all prequesites
# # TODO - it would be nice if the tests could run when this is built
# %.o : %.cpp apple
# 	ECHO "DING DING"
# 	$(CPP_COMP_COM) $< -c -o $@