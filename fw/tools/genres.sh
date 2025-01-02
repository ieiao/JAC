#!/bin/bash

if [ "${0%/*}" = "." ]
then
    echo "Run it at fw directory"
    exit
fi

rm -rf build/res
mkdir -p build/res

echo -n 'processing...'
gcc -c -o build/res/u8g2_fonts.o components/jac/u8g2/csrc/u8g2_fonts.c > /dev/null
./tools/xbm2bin.py resources/tree.xbm > /dev/null
./tools/xbm2bin.py resources/batt-dead.xbm > /dev/null
./tools/4gcov.py resources/moon.xpm > /dev/null
echo 'done'

fonts_list="u8g2_font_logisoso62_tn u8g2_font_logisoso28_tr u8g2_font_wqy16_t_gb2312a u8g2_font_siji_t_6x10 u8g2_font_luIS12_tr u8g2_font_luIS24_tr u8g2_font_luRS10_tr u8g2_font_courR10_tr u8g2_font_helvR10_tr"
offset=0

for font in ${fonts_list}
do
    # echo ${font^^}
    objcopy -O binary -j .rodata.${font} build/res/u8g2_fonts.o build/res/${font}.bin
    s=`stat -c "%s" build/res/${font}.bin`
    echo "#define OFFSET_${font^^} $offset"
    cat build/res/${font}.bin >> build/res/res.bin
    offset=$((offset + s))
done

font="u8g2_font_myfont"
gcc -c -o build/res/${font}.o components/jac/u8g2/csrc/${font}.c > /dev/null
objcopy -O binary -j .rodata.${font} build/res/${font}.o build/res/${font}.bin
s=`stat -c "%s" build/res/${font}.bin`
echo "#define OFFSET_${font^^} $offset"
cat build/res/${font}.bin >> build/res/res.bin
offset=$((offset + s))

for res in `ls resources/*.bin`
do
    # echo ${res^^}
    s=`stat -c "%s" ${res}`
    echo "#define OFFSET_${res^^} $offset" | sed 's/RESOURCES\///g' | sed 's/\./_/g' | sed 's/-/_/g'
    cat ${res} >> build/res/res.bin
    offset=$((offset + s))
done

rm -f resources/*.bin resources/moon.*.xbm build/res/u8g2*
