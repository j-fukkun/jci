#!/bin/bash

try(){
    expected="$1"
    input="$2"

    ./jcc "$input" > tmp.s
    g++ -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
	echo "$input => $actual"
    else
	echo "$input => $expected expected, but got $actual."
	exit 1
    fi   
}


try 2 'return 1+1;'
try 9 'return 10-1;' 
try 15 'return 3*5;' 
try 5 'return 10/2;'
try 18 'return 1+5+4+8;'
try 1 'return 10 == 10;'
try 0 'return 10 == 5;'
try 1 'return 1 != 2;'
try 0 'return 5 != 5;'
try 1 'return 10 < 13;'
try 0 'return 13 < 10;'
try 1 'return 2 <= 2;'
try 1 'return 2 <= 5;'
try 0 'return 8 <= 1;'
try 1 'return 3 > 1;'
try 0 'return 5 > 9;'
try 1 'return 4 >= 4;'
try 1 'return 4 >= 2;'
try 0 'return 3 >= 9;'
try 10 'a = 3+7; return a;'
try 10 'a = 10; hoge = a; return b;'
try 10 'a = 0; b = 0; for(;a < 10; a = a + 1) b = b + 1; return b;'
try 20 'b = 0; while(b < 20) b = b + 1; return b;'
try 5 'a = 0; b = 5; if(b > 10) a = 10; else a = 5; return a;'
echo OK
