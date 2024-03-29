#CXX=g++
CXX=clang++
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
	gcc -g -static -o test/test.out test/test_extern.o test/test.s
	./test/test.out
	./jcc optimization/test/test.c > optimization/test/test.s
	gcc -g -static -o optimization/test/test.out optimization/test/test.s
	./optimization/test/test.out

test_opt: jcc
	./jcc test/test_opt.c > test/test_opt.s
	gcc -g -static -o test/test_opt.out test/test_opt.s
	./test/test_opt.out

nqueen: jcc
	./jcc test/nqueen.c > test/nqueen.s
	gcc -static -o test/nqueen_jcc.out test/nqueen.s
	./test/nqueen_jcc.out

clean:
	rm -f jcc *.o *~ tmp* a.out *.s *.lir *.png *.dot
	rm -f test/*.s test/*~ test/*.o test/test test/*.lir test/*.out test/*.png test/*.dot
	rm -f optimization/*.o optimization/*~

.PHONY: test test_opt nqueen clean
