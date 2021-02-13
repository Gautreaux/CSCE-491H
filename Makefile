CXX:=g++-8 # which compiler to use
CPPFLAGS=# empty default value for GCC flags, can be overriden on command line
CPPSTD=-std=c++17# which c++ version to use

CPP_COMP_COM=$(CXX) $(CPPFLAGS) $(CPPSTD)
CC_COMPLIE_NO_LINK_AUTO=$(CPP_COMP_COM) -c -Wall -o $@ $<
CC_COMPILE_LINK_EXE_AUTO=$(CPP_COMP_COM) -Wall -o $@ $< OFiles/*.o

EXECUTABLE=Main

$(EXECUTABLE) : Main.cpp OFiles/FileUtil.o OFiles/GCodeParser.o OFiles/ChainStar.o
	$(CC_COMPILE_LINK_EXE_AUTO) -lpthread

# declaring a phony forces the top level to always rebuild
.PHONY: $(EXECUTABLE) tests

all : $(EXECUTABLE) tests

force : clean $(EXECUTABLE)

clean :
	rm -f *.o
	rm -f $(EXECUTABLE)
	rm -f OFiles/*.o

release : clean
	make CPPFLAGS=-Ofast

debug : clean
	make CPPFLAGS=-g

# geometry files 
OFiles/GCodeParser.o : GCodeParser.cpp GCodeParser.h OFiles/GCodeSegment.o OFiles/Point3.o OFiles/pch.o
	$(CC_COMPLIE_NO_LINK_AUTO)

OFiles/GCodeSegment.o : GCodeSegment.cpp GCodeSegment.h OFiles/LineSegment.o OFiles/pch.o
	$(CC_COMPLIE_NO_LINK_AUTO)

OFiles/LineSegment.o : GeometryLib/LineSegment.cpp GeometryLib/LineSegment.h OFiles/Line.o OFiles/Point3.o OFiles/pch.o
	$(CC_COMPLIE_NO_LINK_AUTO)

OFiles/Line.o : GeometryLib/Line.cpp GeometryLib/Line.h OFiles/Point3.o OFiles/Vector3.o OFiles/pch.o
	$(CC_COMPLIE_NO_LINK_AUTO)

OFiles/Vector3.o : GeometryLib/Vector3.cpp GeometryLib/Vector3.h OFiles/Point3.o OFiles/pch.o
	$(CC_COMPLIE_NO_LINK_AUTO)

OFiles/Point3.o : GeometryLib/Point3.cpp GeometryLib/Point3.h OFiles/pch.o
	$(CC_COMPLIE_NO_LINK_AUTO)

OFiles/pch.o : pch.cpp pch.h
	mkdir OFiles/ -p
	$(CC_COMPLIE_NO_LINK_AUTO)

# chain* files
OFiles/ChainStar.o : ChainStar.cpp ChainStar.h OFiles/DynamicBitset.o OFiles/GCodeParser.o OFiles/ChainStarHelper.o OFiles/ChainLayerMeta.o OFiles/pch.o
	$(CC_COMPLIE_NO_LINK_AUTO)

OFiles/ChainStarHelper.o : ChainStarHelper.cpp ChainStarHelper.h OFiles/pch.o
	$(CC_COMPLIE_NO_LINK_AUTO)

OFiles/ChainLayerMeta.o : ChainLayerMeta.cpp ChainLayerMeta.h OFiles/pch.o
	$(CC_COMPLIE_NO_LINK_AUTO)

# util files
OFiles/DynamicBitset.o : UtilLib/DynamicBitset.cpp UtilLib/DynamicBitset.h OFiles/pch.o
	$(CC_COMPLIE_NO_LINK_AUTO)

OFiles/BiMap.o : UtilLib/BiMap.cpp UtilLib/BiMap.h OFiles/pch.o
	$(CC_COMPLIE_NO_LINK_AUTO)

OFiles/NonReallocVector.o : UtilLib/NonReallocVector.cpp UtilLib/NonReallocVector.h OFiles/pch.o
	$(CC_COMPLIE_NO_LINK_AUTO)

OFiles/FileUtil.o : UtilLib/FileUtil.cpp UtilLib/FileUtil.h OFiles/pch.o
	$(CC_COMPLIE_NO_LINK_AUTO)

OFiles/ThreadsafeIntGen.o : UtilLib/ThreadsafeIntGen.cpp UtilLib/ThreadsafeIntGen.h OFiles/pch.o
	$(CC_COMPLIE_NO_LINK_AUTO)

# testing files
test : tests
tests : PQTest.exe
	PQTest.exe > test.txt
	python CheckPopPush.py

PQTest.exe : TestingCode/PQTest.cpp OFiles/DynamicBitset.o OFiles/RecomputeState.o OFiles/PrunedAStar.o OFiles/Pch.o
	$(CC_COMPILE_LINK_EXE_AUTO)

PQTest2.exe : TestingCode/PQTest2.cpp OFiles/DynamicBitset.o OFiles/RecomputeState.o OFiles/PrunedAStar.o OFiles/Pch.o
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

#for checking call stacks
# valgrind --tool=callgrind ./Main gcodeSampleSet/81191.gcode 