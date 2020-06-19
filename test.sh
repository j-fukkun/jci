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


try 2 'main(){return 1+1;}'
try 9 'main(){return 10-1;}' 
try 15 'main(){return 3*5;}' 
try 5 'main(){return 10/2;}'
try 18 'main(){return 1+5+4+8;}'
try 1 'main(){return 10 == 10;}'
try 0 'main(){return 10 == 5;}'
try 1 'main(){return 1 != 2;}'
try 0 'main(){return 5 != 5;}'
try 1 'main(){return 10 < 13;}'
try 0 'main(){return 13 < 10;}'
try 1 'main(){return 2 <= 2;}'
try 1 'main(){return 2 <= 5;}'
try 0 'main(){return 8 <= 1;}'
try 1 'main(){return 3 > 1;}'
try 0 'main(){return 5 > 9;}'
try 1 'main(){return 4 >= 4;}'
try 1 'main(){return 4 >= 2;}'
try 0 'main(){return 3 >= 9;}'
try 10 'main(){a = 3+7; return a;}'
try 10 'main(){a = 10; hoge = a; return hoge;}'
try 10 'main(){a = 0; b = 0; for(;a < 10; a = a + 1) b = b + 1; return b;}'
try 20 'main(){b = 0; while(b < 20) b = b + 1; return b;}'
try 10 'main(){a = 0; b = 5; if(b < 10) a = 10; else a = 5; return a;}'
try 5 'main(){a = 0; b = 5; if(b > 10) a = 10; else a = 5; return a;}'
try 20 'main(){a = 0; b = 5; c = 0; if(b < 10){ a = 10; c = 20;} else {a = 5; c = 30;} return c;}'
try 24 'kaijo(a){ if(a == 0) return 1; b = a * kaijo(a-1); return b;} main(){return kaijo(4);}'
try 55 'fibo(a){if(a == 0) return 0; else if(a == 1) return 1; else return (fibo(a-1)+fibo(a-2));} main(){return fibo(10);}'
try 10 'main(){a = 10; b = &a; return *b;}'
echo OK
