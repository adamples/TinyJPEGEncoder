#!/bin/sh

make clean -C ../..
rm -f -- "../../mode.inc"
ln -s "profile.inc" "../../mode.inc"
make all -C ../..
sh gprof2png.sh ../convert random.bmp output_tiny.jpeg
rm -f -- output_tiny.jpeg
