CXX=g++
CXXFLAGS=-std=c++11 -g -static
SRCS=$(wildcard *.cpp)
OBJS=$(SRCS:.cpp=.o)

jcc: $(OBJS)
	$(CXX) -o jcc $(OBJS) $(CXXFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: jcc 
	gcc -c test/inc.c -o test/inc.o
	./jcc test/test.c > test/test.s
	gcc -o test/test test/inc.o test/test.s
	test/test

clean:
	rm -f jcc *.o *~ tmp* a.out *.s
	rm -f test/*.s test/*~

.PHONY: test clean
