#!/bin/sh

input="$1.dot"
output="$1.png"

dot -Tpng $input -o $output
