#!/bin/sh

$@
gprof "$1" gmon.out > gmon.txt
python gprof2dot.py gmon.txt | dot -Tpng -o gprof.png
