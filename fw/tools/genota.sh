#!/bin/bash

set -e
ib1=build/res/res.bin
ib2=build/storage.bin
ib3=build/JAC.bin
ob=build/JAC_ota.bin

rm -f $ob

# res
# id
printf "%08x" 1 | sed -E 's/(..)/\1\n/g' | tac | tr '\n' ' ' | sed 's/ //g' | xxd -r -p >> $ob
# size
# padding to align 512 bytes
os=`stat -c%s $ib1`
ns=$((os + 511))
ns=$((ns / 512))
ns=$((ns * 512))
printf "%08x" $ns | sed -E 's/(..)/\1\n/g' | tac | tr '\n' ' ' | sed 's/ //g' | xxd -r -p >> $ob
# padding to make sure label align 512 bytes
dd if=/dev/zero bs=504 count=1 2>/dev/null | tr '\000' '\377' >> $ob
# firmware
cat $ib1 >> $ob
padding=$((ns - os))
dd if=/dev/zero bs=$padding count=1 2>/dev/null | tr '\000' '\377' >> $ob

# storage
# id
printf "%08x" 2 | sed -E 's/(..)/\1\n/g' | tac | tr '\n' ' ' | sed 's/ //g' | xxd -r -p >> $ob
# size
# padding to align 512 bytes
os=`stat -c%s $ib2`
ns=$((os + 511))
ns=$((ns / 512))
ns=$((ns * 512))
printf "%08x" $ns | sed -E 's/(..)/\1\n/g' | tac | tr '\n' ' ' | sed 's/ //g' | xxd -r -p >> $ob
# padding to make sure label align 512 bytes
dd if=/dev/zero bs=504 count=1 2>/dev/null | tr '\000' '\377' >> $ob
# firmware
cat $ib2 >> $ob
padding=$((ns - os))
dd if=/dev/zero bs=$padding count=1 2>/dev/null | tr '\000' '\377' >> $ob

# app
# id
printf "%08x" 3 | sed -E 's/(..)/\1\n/g' | tac | tr '\n' ' ' | sed 's/ //g' | xxd -r -p >> $ob
# size
# padding to align 512 bytes
os=`stat -c%s $ib3`
ns=$((os + 511))
ns=$((ns / 512))
ns=$((ns * 512))
printf "%08x" $ns | sed -E 's/(..)/\1\n/g' | tac | tr '\n' ' ' | sed 's/ //g' | xxd -r -p >> $ob
# padding to make sure label align 512 bytes
dd if=/dev/zero bs=504 count=1 2>/dev/null | tr '\000' '\377' >> $ob
# firmware
cat $ib3 >> $ob
padding=$((ns - os))
dd if=/dev/zero bs=$padding count=1 2>/dev/null | tr '\000' '\377' >> $ob

