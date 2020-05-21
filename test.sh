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

echo OK
