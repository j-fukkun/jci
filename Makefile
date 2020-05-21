CFLAGS=-std=c++11 -g -static

jcc: main.cpp
	g++ $(CFLAGS) main.cpp -o jcc

test: jcc
	sh test.sh

clean:
	rm -f jcc *.o *~ tmp*

.PHONY: test clean
