#!/bin/bash

if [ "${0%/*}" = "." ]
then
    echo "Run it at fw directory"
    exit
fi

rm -rf build/fonts
mkdir -p build/fonts
touch build/fonts/fonts.bin
gcc -c -o build/fonts/u8g2_fonts.o components/u8g2/csrc/u8g2_fonts.c

fonts_list="u8g2_font_logisoso62_tn u8g2_font_logisoso28_tn u8g2_font_wqy16_t_gb2312a u8g2_font_siji_t_6x10"
offset=0

for font in ${fonts_list}
do
#    echo ${font^^}
    objcopy -O binary -j .rodata.${font} build/fonts/u8g2_fonts.o build/fonts/${font}.bin
    s=`stat -c "%s" build/fonts/${font}.bin`
    echo "#define OFFSET_${font^^} $offset"
    cat build/fonts/${font}.bin >> build/fonts/fonts.bin
    offset=$((offset + s))
done
