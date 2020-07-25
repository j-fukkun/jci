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


try 2 'int main(){return 1+1;}'
try 9 'int main(){return 10-1;}' 
try 15 'int main(){return 3*5;}' 
try 5 'int main(){return 10/2;}'
try 18 'int main(){return 1+5+4+8;}'
try 1 'int main(){return 10 == 10;}'
try 0 'int main(){return 10 == 5;}'
try 1 'int main(){return 1 != 2;}'
try 0 'int main(){return 5 != 5;}'
try 1 'int main(){return 10 < 13;}'
try 0 'int main(){return 13 < 10;}'
try 1 'int main(){return 2 <= 2;}'
try 1 'int main(){return 2 <= 5;}'
try 0 'int main(){return 8 <= 1;}'
try 1 'int main(){return 3 > 1;}'
try 0 'int main(){return 5 > 9;}'
try 1 'int main(){return 4 >= 4;}'
try 1 'int main(){return 4 >= 2;}'
try 0 'int main(){return 3 >= 9;}'
try 10 'int main(){int a; a = 3+7; return a;}'
try 10 'int main(){int a; a = 10; int hoge; hoge = a; return hoge;}'
try 10 'int main(){int a; a = 0; int b; b = 0; for(;a < 10; a = a + 1) b = b + 1; return b;}'
try 20 'int main(){int b; b = 0; while(b < 20) b = b + 1; return b;}'
try 10 'int main(){int a; a = 0; int b; b = 5; if(b < 10) a = 10; else a = 5; return a;}'
try 5 'int main(){int a; a = 0; int b; b = 5; if(b > 10) a = 10; else a = 5; return a;}'
try 20 'int main(){int a; a = 0; int b; b = 5; int c; c = 0; if(b < 10){ a = 10; c = 20;} else {a = 5; c = 30;} return c;}'
try 24 'int kaijo(int a){ if(a == 0) return 1; int b; b = a * kaijo(a-1); return b;} int main(){return kaijo(4);}'
try 55 'int fibo(int a){if(a == 0) return 0; else if(a == 1) return 1; else return (fibo(a-1)+fibo(a-2));} int main(){return fibo(10);}'
try 10 'int main(){int a; a = 10; int* b; b = &a; return *b;}'
try 4 'int main(){int a; return sizeof(a);}'
try 8 'int main(){int* x; return sizeof(x);}'
try 4 'int main(){int x; return sizeof(x+3);}'
try 4 'int main(){return sizeof(1);}'
try 4 'int main(){return sizeof(sizeof(5));}'
try 3 'int main(){int a[1+1]; *a=1; *(a+1)=2; int* p; p=a; return *p + *(p+1);}'
try 3 'int main(){int a[1+1]; *a=1; *(a+1)=2; int* p; p=(a+1); return *p + *(p-1);}'
try 5 'int main(){int a[2]; a[0] = 2; a[1] = 3; return a[0]+a[1];}'
try 8 'int x; int main(){x = 8; return x;}'
try 1 'int x; int main(){int x; x = 1; return x;}'
try 5 'int x; int main(){x = 2; int y; y = 3; return x+y;}'
try 5 'int main(){char a; char c; a = 10; c = 5; return a - c;}'
try 5 'int main(){char a[2]; a[0] = 2; a[1] = 3; return a[0] + a[1];}'
try 48 'int main(){char a[3]; a[0] = "a"; a[1] = "b"; a[2] = "c"; return a[2];}'

echo OK
