#!/bin/sh

INPUT_PATH="$1"
OUTPUT_PATH="$2"

REFERENCE_MATRIX=" \
  16   11   10   16   24   40   51   61 \
  12   12   14   19   26   58   60   55 \
  14   13   16   24   40   57   69   56 \
  14   17   22   29   51   87   80   62 \
  18   22   37   56   68  109  103   77 \
  24   35   55   64   81  104  113   92 \
  49   64   78   87  103  121  120  101 \
  72   92   95   98  112  100  103   99"

REFERENCE_HEADER_MATRIX=" \
  16   11   12   14   12   10   16   14 \
  13   14   18   17   16   19   24   40 \
  26   24   22   22   24   49   35   37 \
  29   40   58   51   61   60   57   51 \
  56   55   64   72   92   78   64   68 \
  87   69   55   56   80  109   81   87 \
  95   98  103  104  103   62   77  113 \
 121  112  100  120   92  101  103   99"

Q=$(grep "TJPEG_QUALITY" "$INPUT_PATH" | egrep -o '[0-9]+')
OUTPUT_MATRIX=""

if [ "$Q" "<" "50" ]; then
  S=$(expr "5000" "/" "$Q")
else
  S=$(expr "2" "*" "$Q")
  S=$(expr "200" "-" "$S")
fi

touch "$OUTPUT_PATH"
truncate -s0 "$OUTPUT_PATH"
echo "#define QUANTIZATION_MATRIX_DATA \\" >> "$OUTPUT_PATH"

I=0
for X in $REFERENCE_MATRIX; do
  Y=$(expr "$S" "*" "$X" "+" "50")
  Y=$(expr "$Y" "/" "100")
  printf "%3d," "$Y" >> "$OUTPUT_PATH"
  I=$(expr "$I" "+" "1")
  if [ "$I" "=" "8" ]; then
    I=0
    echo " \\" >> "$OUTPUT_PATH"
  else
    echo -n " " >> "$OUTPUT_PATH"
  fi
done

truncate -s-4 "$OUTPUT_PATH"
echo >> "$OUTPUT_PATH"

echo "#define QUANTIZATION_MATRIX_HEADER_DATA \\" >> "$OUTPUT_PATH"

I=0
for X in $REFERENCE_HEADER_MATRIX; do
  Y=$(expr "$S" "*" "$X" "+" "50")
  Y=$(expr "$Y" "/" "100")
  printf "%3d," "$Y" >> "$OUTPUT_PATH"
  I=$(expr "$I" "+" "1")
  if [ "$I" "=" "8" ]; then
    I=0
    echo " \\" >> "$OUTPUT_PATH"
  else
    echo -n " " >> "$OUTPUT_PATH"
  fi
done

truncate -s-4 "$OUTPUT_PATH"
echo >> "$OUTPUT_PATH"
