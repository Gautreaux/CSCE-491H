CXX:=g++ # which compiler to use
CPPFLAGS=# empty default value for GCC flags
CPPSTD=-std=c++11# which c++ version to use

CPP_COMP_COM=$(CXX) $(CPPFLAGS) $(CPPSTD)
CPP_LINK_ALL_REQUIRED=$(CPP_COMP_COM) -c -o $@ $<

EXECUTABLE=Main.exe

.PHONY: $(EXECUTABLE)

all : $(EXECUTABLE)

force : clean $(EXECUTABLE)

clean :
	del /S *.o

manual_all :
	g++ -g -DDEBUG -o $(EXECUTABLE) Main.cpp GCodeSegment.cpp GCodeParser.cpp GeometryLib/Line.cpp GeometryLib/LineSegment.cpp GeometryLib/Point3.cpp GeometryLib/Vector3.cpp

# echo "BUILD TRIGGERED (cpp_impl)"
# echo $(CPPFLAGS)
$(EXECUTABLE) : Main.o GCodeParser.o
	$(CPP_COMP_COM) -o $(EXECUTABLE) *.o

GCodeParser.o : GCodeParser.cpp GCodeParser.h GCodeSegment.o Point3.o pch.o
	$(CPP_LINK_ALL_REQUIRED)

GCodeSegment.o : GCodeSegment.cpp GCodeSegment.h LineSegment.o pch.o
	$(CPP_LINK_ALL_REQUIRED)

LineSegment.o : GeometryLib/LineSegment.cpp GeometryLib/LineSegment.h Line.o Point3.o pch.o
	$(CPP_LINK_ALL_REQUIRED)

Line.o : GeometryLib/Line.cpp GeometryLib/Line.h Point3.o Vector3.o pch.o
	$(CPP_LINK_ALL_REQUIRED)

Vector3.o : GeometryLib/Vector3.cpp GeometryLib/Vector3.h Point3.o pch.o
	$(CPP_LINK_ALL_REQUIRED)

Point3.o : GeometryLib/Point3.cpp GeometryLib/Point3.h pch.o
	$(CPP_LINK_ALL_REQUIRED)

pch.o : pch.cpp pch.h
	$(CPP_LINK_ALL_REQUIRED)

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