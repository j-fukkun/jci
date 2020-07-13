CXX=g++
CXXFLAGS=-std=c++11 -g -static
SRCS=$(wildcard *.cpp)
OBJS=$(SRCS:.cpp=.o)

jcc: $(OBJS)
	$(CXX) -o jcc $(OBJS) $(CXXFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: jcc
	sh test.sh

clean:
	rm -f jcc *.o *~ tmp* a.out

.PHONY: test clean
