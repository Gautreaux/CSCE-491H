CXX:=g++-8 # which compiler to use
CPPFLAGS=# empty default value for GCC flags, can be overriden on command line
CPPSTD=-std=c++17# which c++ version to use

CPP_COMP_COM=$(CXX) $(CPPFLAGS) $(CPPSTD)
CC_COMPLIE_NO_LINK_AUTO=$(CPP_COMP_COM) -c -Wall -o $@ $<
CC_COMPILE_LINK_EXE_AUTO=$(CPP_COMP_COM) -o $@ $< *.o

EXECUTABLE=Main.exe

$(EXECUTABLE) : Main.cpp FileUtil.o GCodeParser.o PrunedAStar.o
	$(CC_COMPILE_LINK_EXE_AUTO)

# declaring a phony forces the top level to always rebuild
.PHONY: $(EXECUTABLE) tests

all : $(EXECUTABLE) tests

force : clean $(EXECUTABLE)

clean :
	rm -f *.o
	rm -f *.exe
	# del /S *.o
	# del /S *.exe


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
PrunedAStar.o : PrunedAStar.cpp PrunedAStar.h RecomputeState.o GCodeParser.o BiMap.o LayerManager.o NonReallocVector.o PQWrapper.o pch.o
	$(CC_COMPLIE_NO_LINK_AUTO)

RecomputeState.o : RecomputeState.cpp RecomputeState.h DynamicBitset.o Point3.o LayerManager.o pch.o
	$(CC_COMPLIE_NO_LINK_AUTO)

LayerManager.o : LayerManager.cpp LayerManager.h GCodeParser.o GCodeSegment.o Point3.o BiMap.o pch.o
	$(CC_COMPLIE_NO_LINK_AUTO)

PQWrapper.o : PQWrapper.cpp PQWrapper.h RecomputeState.o pch.o
	$(CC_COMPLIE_NO_LINK_AUTO)

# util files
DynamicBitset.o : UtilLib/DynamicBitset.cpp UtilLib/DynamicBitset.h pch.o
	$(CC_COMPLIE_NO_LINK_AUTO)

BiMap.o : UtilLib/BiMap.cpp UtilLib/BiMap.h pch.o
	$(CC_COMPLIE_NO_LINK_AUTO)

NonReallocVector.o : UtilLib/NonReallocVector.cpp UtilLib/NonReallocVector.h pch.o
	$(CC_COMPLIE_NO_LINK_AUTO)

FileUtil.o : UtilLib/FileUtil.cpp UtilLib/FileUtil.h pch.o
	$(CC_COMPLIE_NO_LINK_AUTO)

# testing files
test : tests
tests : PQTest.exe
	PQTest.exe > test.txt
	python CheckPopPush.py

PQTest.exe : TestingCode/PQTest.cpp DynamicBitset.o RecomputeState.o PrunedAStar.o Pch.o
	$(CC_COMPILE_LINK_EXE_AUTO)

PQTest2.exe : TestingCode/PQTest2.cpp DynamicBitset.o RecomputeState.o PrunedAStar.o Pch.o
	$(CC_COMPILE_LINK_EXE_AUTO)


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