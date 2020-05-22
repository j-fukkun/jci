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

try 0 0
try 42 42
try 2 '1+1'
try 9 '10-1' 
try 15 '3*5' 
try 5 '10/2'
try 18 '1+5+4+8'
try 1 '10 == 10'
try 0 '10 == 5'
try 1 '1 != 2'
try 0 '5 != 5'
try 1 '10 < 13'
try 0 '13 < 10'
try 1 '2 <= 2'
try 1 '2 <= 5'
try 0 '8 <= 1'
try 1 '3 > 1'
try 0 '5 > 9'
try 1 '4 >= 4'
try 1 '4 >= 2'
try 0 '3 >= 9'

echo OK
