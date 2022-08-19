CXX=g++
CXXFLAGS=-std=c++11 -g -static
SRCS=$(wildcard *.cpp)
OBJS=$(SRCS:.cpp=.o)

jcc: $(OBJS)
	$(CXX) -o jcc $(OBJS) $(CXXFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: jcc 
	gcc -no-pie -c test/inc.c -o test/inc.o
	gcc -xc -c test/test_extern.c -o test/test_extern.o
	./jcc test/test.c > test/test.s
	gcc -no-pie -o test/test test/inc.o test/test_extern.o test/test.s
	test/test

clean:
	rm -f jcc *.o *~ tmp* a.out *.s
	rm -f test/*.s test/*~ test/*.o test/test

.PHONY: test clean
