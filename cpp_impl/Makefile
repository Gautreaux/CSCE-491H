CXX:=g++ # which compiler to use
CPPFLAGS=# empty default value for GCC flags
CPPSTD=-std=c++11# which c++ version to use

CPP_COMP_COM=$(CXX) $(CPPFLAGS) $(CPPSTD)

EXECUTABLE=Main.exe

.PHONY: $(EXECUTABLE)

all : clean $(EXECUTABLE)

clean : 
	del TestFunction.o

$(EXECUTABLE) : TestFunction.o
	echo "BUILD TRIGGERED (cpp_impl)"
	echo $(CPPFLAGS)
	$(CPP_COMP_COM) Main.cpp $^ -o $(EXECUTABLE)

# Uses builtin .cpp -> .o translation instead
#	which is basically this rule anyway
# # automatic rule for building .o files from same name .cpp files
# # $@ - the name of the target of the rule
# # $< - the name of the first prequesite
# # TODO - it would be nice if the tests could run when this is built
# %.o : %.cpp apple
# 	ECHO "DING DING"
# 	$(CPP_COMP_COM) $< -c -o $@