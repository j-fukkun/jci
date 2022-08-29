CXX=g++
CXXFLAGS=-std=c++11 -g 
SRCS=$(wildcard *.cpp)
SRCS+=$(wildcard ./optimization/*.cpp)
OBJS=$(SRCS:.cpp=.o)

jcc: $(OBJS)
	$(CXX) -o jcc $(OBJS) $(CXXFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: jcc 
	gcc -xc -c test/test_extern.c -o test/test_extern.o
	./jcc test/test.c > test/test.s
	gcc -static -o test/test.out test/test_extern.o test/test.s
	./test/test.out

test_opt: jcc
	./jcc test/test_opt.c > test/test_opt.s
	gcc -static -o test/test_opt.out test/test_opt.s
	./test/test_opt.out

clean:
	rm -f jcc *.o *~ tmp* a.out *.s *.lir
	rm -f test/*.s test/*~ test/*.o test/test test/*.lir test/*.out
	rm -f optimization/*.o optimization/*~

.PHONY: test clean
