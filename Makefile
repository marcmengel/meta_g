CXX=g++
CXXFLAGS=-g 

all: impl_test.out states_test.out

impl_test.out: impl_test impl_test.txt
	./impl_test < impl_test.txt 2>&1 | tee impl_test.out

states_test.out: states_test states_test.txt
	./states_test < states_test.txt 2>&1 | tee states_test.out

states_test: states.cc tokenmap.o
	$(CXX) $(CXXFLAGS) -fpermissive -o $@ -DDEBUG=1 -DUNITTEST=1 states.cc tokenmap.o

tokenmap.cc: tokenmap.sh tokens.h
	sh tokenmap.sh > tokenmap.cc

impl_test: impl.cc states.o tokenmap.o
	$(CXX) $(CXXFLAGS) -fpermissive -o $@ -DDEBUG=1 -DUNITTEST=1 impl.cc  states.o tokenmap.o

