#!/bin/bash

if [ "${0%/*}" = "." ]
then
    echo "Run it at fw directory"
    exit
fi

rm -rf build/res
mkdir -p build/res

./tools/xbm2bin.py resources/tree.xbm
./tools/xbm2bin.py resources/batt-dead.xbm
./tools/4gcov.py resources/moon.xpm

mv resources/*.bin build/res/

offset=0
for res in `ls build/res/`
do
#    echo ${res^^}
    s=`stat -c "%s" build/res/${res}`
    echo "#define OFFSET_${res^^} $offset" | sed 's/\./_/g' | sed 's/-/_/g'
    cat build/res/${res} >> build/res/res.bin
    offset=$((offset + s))
done
